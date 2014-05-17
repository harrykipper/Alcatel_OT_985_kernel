

#include "config.h"
#include "JNIUtilityPrivate.h"

#if ENABLE(MAC_JAVA_BRIDGE)

#include "JNIBridgeJSC.h"
#include "runtime_array.h"
#include "runtime_object.h"
#include <runtime/JSArray.h>
#include <runtime/JSLock.h>

namespace JSC {

namespace Bindings {

static jobject convertArrayInstanceToJavaArray(ExecState* exec, JSArray* jsArray, const char* javaClassName)
{
    JNIEnv* env = getJNIEnv();
    // As JS Arrays can contain a mixture of objects, assume we can convert to
    // the requested Java Array type requested, unless the array type is some object array
    // other than a string.
    unsigned length = jsArray->length();
    jobjectArray jarray = 0;

    // Build the correct array type
    switch (JNITypeFromPrimitiveType(javaClassName[1])) {
    case object_type:
            {
            // Only support string object types
            if (!strcmp("[Ljava.lang.String;", javaClassName)) {
                jarray = (jobjectArray)env->NewObjectArray(length,
                    env->FindClass("java/lang/String"),
                    env->NewStringUTF(""));
                for (unsigned i = 0; i < length; i++) {
                    JSValue item = jsArray->get(exec, i);
                    UString stringValue = item.toString(exec);
                    env->SetObjectArrayElement(jarray, i,
                        env->functions->NewString(env, (const jchar *)stringValue.data(), stringValue.size()));
                }
            }
            break;
        }

    case boolean_type:
        {
            jarray = (jobjectArray)env->NewBooleanArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jboolean value = (jboolean)item.toNumber(exec);
                env->SetBooleanArrayRegion((jbooleanArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case byte_type:
        {
            jarray = (jobjectArray)env->NewByteArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jbyte value = (jbyte)item.toNumber(exec);
                env->SetByteArrayRegion((jbyteArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case char_type:
        {
            jarray = (jobjectArray)env->NewCharArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                UString stringValue = item.toString(exec);
                jchar value = 0;
                if (stringValue.size() > 0)
                    value = ((const jchar*)stringValue.data())[0];
                env->SetCharArrayRegion((jcharArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case short_type:
        {
            jarray = (jobjectArray)env->NewShortArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jshort value = (jshort)item.toNumber(exec);
                env->SetShortArrayRegion((jshortArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case int_type:
        {
            jarray = (jobjectArray)env->NewIntArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jint value = (jint)item.toNumber(exec);
                env->SetIntArrayRegion((jintArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case long_type:
        {
            jarray = (jobjectArray)env->NewLongArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jlong value = (jlong)item.toNumber(exec);
                env->SetLongArrayRegion((jlongArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case float_type:
        {
            jarray = (jobjectArray)env->NewFloatArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jfloat value = (jfloat)item.toNumber(exec);
                env->SetFloatArrayRegion((jfloatArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case double_type:
        {
            jarray = (jobjectArray)env->NewDoubleArray(length);
            for (unsigned i = 0; i < length; i++) {
                JSValue item = jsArray->get(exec, i);
                jdouble value = (jdouble)item.toNumber(exec);
                env->SetDoubleArrayRegion((jdoubleArray)jarray, (jsize)i, (jsize)1, &value);
            }
            break;
        }

    case array_type: // don't handle embedded arrays
    case void_type: // Don't expect arrays of void objects
    case invalid_type: // Array of unknown objects
        break;
    }

    // if it was not one of the cases handled, then null is returned
    return jarray;
}

jvalue convertValueToJValue(ExecState* exec, JSValue value, JNIType jniType, const char* javaClassName)
{
    JSLock lock(SilenceAssertionsOnly);

    jvalue result;

    switch (jniType) {
    case array_type:
    case object_type:
        {
            result.l = (jobject)0;

            // First see if we have a Java instance.
            if (value.isObject()) {
                JSObject* objectImp = asObject(value);
                if (objectImp->classInfo() == &RuntimeObjectImp::s_info) {
                    RuntimeObjectImp* imp = static_cast<RuntimeObjectImp*>(objectImp);
                    JavaInstance* instance = static_cast<JavaInstance*>(imp->getInternalInstance());
                    if (instance)
                        result.l = instance->javaInstance();
                } else if (objectImp->classInfo() == &RuntimeArray::s_info) {
                    // Input is a JavaScript Array that was originally created from a Java Array
                    RuntimeArray* imp = static_cast<RuntimeArray*>(objectImp);
                    JavaArray* array = static_cast<JavaArray*>(imp->getConcreteArray());
                    result.l = array->javaArray();
                } else if (objectImp->classInfo() == &JSArray::info) {
                    // Input is a Javascript Array. We need to create it to a Java Array.
                    result.l = convertArrayInstanceToJavaArray(exec, asArray(value), javaClassName);
                }
            }

            // Now convert value to a string if the target type is a java.lang.string, and we're not
            // converting from a Null.
            if (!result.l && !strcmp(javaClassName, "java.lang.String")) {
#ifdef CONVERT_NULL_TO_EMPTY_STRING
                if (value->isNull()) {
                    JNIEnv* env = getJNIEnv();
                    jchar buf[2];
                    jobject javaString = env->functions->NewString(env, buf, 0);
                    result.l = javaString;
                } else
#else
                if (!value.isNull())
#endif
                {
                    UString stringValue = value.toString(exec);
                    JNIEnv* env = getJNIEnv();
                    jobject javaString = env->functions->NewString(env, (const jchar *)stringValue.data(), stringValue.size());
                    result.l = javaString;
                }
            } else if (!result.l)
                // ANDROID
                memset(&result, 0, sizeof(jvalue)); // Handle it the same as a void case
        }
        break;

    case boolean_type:
        {
            result.z = (jboolean)value.toNumber(exec);
        }
        break;

    case byte_type:
        {
            result.b = (jbyte)value.toNumber(exec);
        }
        break;

    case char_type:
        {
            result.c = (jchar)value.toNumber(exec);
        }
        break;

    case short_type:
        {
            result.s = (jshort)value.toNumber(exec);
        }
        break;

    case int_type:
        {
            result.i = (jint)value.toNumber(exec);
        }
        break;

    case long_type:
        {
            result.j = (jlong)value.toNumber(exec);
        }
        break;

    case float_type:
        {
            result.f = (jfloat)value.toNumber(exec);
        }
        break;

    case double_type:
        {
            result.d = (jdouble)value.toNumber(exec);
        }
        break;

        break;

    case invalid_type:
    default:
    case void_type:
        {
            // ANDROID
            memset(&result, 0, sizeof(jvalue));
        }
        break;
    }
    return result;
}

} // end of namespace Bindings

} // end of namespace JSC

#endif // ENABLE(MAC_JAVA_BRIDGE)
