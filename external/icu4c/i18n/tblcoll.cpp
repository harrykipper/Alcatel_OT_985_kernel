

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

#include "unicode/tblcoll.h"
#include "unicode/coleitr.h"
#include "unicode/ures.h"
#include "unicode/uset.h"
#include "ucol_imp.h"
#include "uresimp.h"
#include "uhash.h"
#include "cmemory.h"
#include "cstring.h"
#include "putilimp.h"

/* public RuleBasedCollator constructor ---------------------------------- */

U_NAMESPACE_BEGIN

RuleBasedCollator::RuleBasedCollator(const RuleBasedCollator& that)
: Collator(that)
, dataIsOwned(FALSE)
, isWriteThroughAlias(FALSE)
, ucollator(NULL)
{
    RuleBasedCollator::operator=(that);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     UErrorCode& status) :
dataIsOwned(FALSE)
{
    construct(rules,
        UCOL_DEFAULT_STRENGTH,
        UCOL_DEFAULT,
        status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     ECollationStrength collationStrength,
                                     UErrorCode& status) : dataIsOwned(FALSE)
{
    construct(rules,
        getUCollationStrength(collationStrength),
        UCOL_DEFAULT,
        status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     UColAttributeValue decompositionMode,
                                     UErrorCode& status) :
dataIsOwned(FALSE)
{
    construct(rules,
        UCOL_DEFAULT_STRENGTH,
        decompositionMode,
        status);
}

RuleBasedCollator::RuleBasedCollator(const UnicodeString& rules,
                                     ECollationStrength collationStrength,
                                     UColAttributeValue decompositionMode,
                                     UErrorCode& status) : dataIsOwned(FALSE)
{
    construct(rules,
        getUCollationStrength(collationStrength),
        decompositionMode,
        status);
}
RuleBasedCollator::RuleBasedCollator(const uint8_t *bin, int32_t length,
                    const RuleBasedCollator *base,
                    UErrorCode &status) :
dataIsOwned(TRUE),
isWriteThroughAlias(FALSE)
{
  ucollator = ucol_openBinary(bin, length, base->ucollator, &status);
}

void
RuleBasedCollator::setRuleStringFromCollator()
{
    int32_t length;
    const UChar *r = ucol_getRules(ucollator, &length);

    if (r && length > 0) {
        // alias the rules string
        urulestring.setTo(TRUE, r, length);
    }
    else {
        urulestring.truncate(0); // Clear string.
    }
}

// not aliasing, not write-through
void
RuleBasedCollator::construct(const UnicodeString& rules,
                             UColAttributeValue collationStrength,
                             UColAttributeValue decompositionMode,
                             UErrorCode& status)
{
    ucollator = ucol_openRules(rules.getBuffer(), rules.length(),
        decompositionMode, collationStrength,
        NULL, &status);

    dataIsOwned = TRUE; // since we own a collator now, we need to get rid of it
    isWriteThroughAlias = FALSE;

    if(ucollator == NULL) {
        if(U_SUCCESS(status)) {
            status = U_MEMORY_ALLOCATION_ERROR;
        }
        return; // Failure
    }

    setRuleStringFromCollator();
}

/* RuleBasedCollator public destructor ----------------------------------- */

RuleBasedCollator::~RuleBasedCollator()
{
    if (dataIsOwned)
    {
        ucol_close(ucollator);
    }
    ucollator = 0;
}

/* RuleBaseCollator public methods --------------------------------------- */

UBool RuleBasedCollator::operator==(const Collator& that) const
{
  /* only checks for address equals here */
  if (Collator::operator==(that))
    return TRUE;

  if (getDynamicClassID() != that.getDynamicClassID())
    return FALSE;  /* not the same class */

  RuleBasedCollator& thatAlias = (RuleBasedCollator&)that;

  // weiv: use C function, commented code below is wrong
  return ucol_equals(this->ucollator, thatAlias.ucollator);
  /*
  synwee : orginal code does not check for data compatibility
  */
  /*
  if (ucollator != thatAlias.ucollator)
    return FALSE;

  return TRUE;
  */
}

UBool RuleBasedCollator::operator!=(const Collator& other) const
{
    return !(*this == other);
}

// aliasing, not write-through
RuleBasedCollator& RuleBasedCollator::operator=(const RuleBasedCollator& that)
{
    if (this != &that)
    {
        if (dataIsOwned)
        {
            ucol_close(ucollator);
        }

        urulestring.truncate(0); // empty the rule string
        dataIsOwned = TRUE;
        isWriteThroughAlias = FALSE;

        UErrorCode intStatus = U_ZERO_ERROR;
        int32_t buffersize = U_COL_SAFECLONE_BUFFERSIZE;
        ucollator = ucol_safeClone(that.ucollator, NULL, &buffersize,
                                        &intStatus);
        if (U_SUCCESS(intStatus)) {
            setRuleStringFromCollator();
        }
    }
    return *this;
}

// aliasing, not write-through
Collator* RuleBasedCollator::clone() const
{
    return new RuleBasedCollator(*this);
}

CollationElementIterator* RuleBasedCollator::createCollationElementIterator
                                           (const UnicodeString& source) const
{
    UErrorCode status = U_ZERO_ERROR;
    CollationElementIterator *result = new CollationElementIterator(source, this,
                                                                    status);
    if (U_FAILURE(status)) {
        delete result;
        return NULL;
    }

    return result;
}

CollationElementIterator* RuleBasedCollator::createCollationElementIterator
                                       (const CharacterIterator& source) const
{
    UErrorCode status = U_ZERO_ERROR;
    CollationElementIterator *result = new CollationElementIterator(source, this,
                                                                    status);

    if (U_FAILURE(status)) {
        delete result;
        return NULL;
    }

    return result;
}

const UnicodeString& RuleBasedCollator::getRules() const
{
    return urulestring;
}

void RuleBasedCollator::getRules(UColRuleOption delta, UnicodeString &buffer)
{
    int32_t rulesize = ucol_getRulesEx(ucollator, delta, NULL, -1);

    if (rulesize > 0) {
        UChar *rules = (UChar*) uprv_malloc( sizeof(UChar) * (rulesize) );
        if(rules != NULL) {
            ucol_getRulesEx(ucollator, delta, rules, rulesize);
            buffer.setTo(rules, rulesize);
            uprv_free(rules);
        } else { // couldn't allocate
            buffer.remove();
        }
    }
    else {
        buffer.remove();
    }
}

UnicodeSet *
RuleBasedCollator::getTailoredSet(UErrorCode &status) const
{
    if(U_FAILURE(status)) {
        return NULL;
    }
    return (UnicodeSet *)ucol_getTailoredSet(this->ucollator, &status);
}


void RuleBasedCollator::getVersion(UVersionInfo versionInfo) const
{
    if (versionInfo!=NULL){
        ucol_getVersion(ucollator, versionInfo);
    }
}

Collator::EComparisonResult RuleBasedCollator::compare(
                                               const UnicodeString& source,
                                               const UnicodeString& target,
                                               int32_t length) const
{
    UErrorCode status = U_ZERO_ERROR;
    return getEComparisonResult(compare(source.getBuffer(), uprv_min(length,source.length()), target.getBuffer(), uprv_min(length,target.length()), status));
}

UCollationResult RuleBasedCollator::compare(
                                               const UnicodeString& source,
                                               const UnicodeString& target,
                                               int32_t length,
                                               UErrorCode &status) const
{
    return compare(source.getBuffer(), uprv_min(length,source.length()), target.getBuffer(), uprv_min(length,target.length()), status);
}

Collator::EComparisonResult RuleBasedCollator::compare(const UChar* source,
                                                       int32_t sourceLength,
                                                       const UChar* target,
                                                       int32_t targetLength)
                                                       const
{
    return  getEComparisonResult(ucol_strcoll(ucollator, source, sourceLength,
                                                         target, targetLength));
}

UCollationResult RuleBasedCollator::compare(const UChar* source,
                                                       int32_t sourceLength,
                                                       const UChar* target,
                                                       int32_t targetLength,
                                                       UErrorCode &status) const
{
    if(U_SUCCESS(status)) {
        return  ucol_strcoll(ucollator, source, sourceLength, target, targetLength);
    } else {
        return UCOL_EQUAL;
    }
}

Collator::EComparisonResult RuleBasedCollator::compare(
                                             const UnicodeString& source,
                                             const UnicodeString& target) const
{
    return getEComparisonResult(ucol_strcoll(ucollator, source.getBuffer(), source.length(),
                                                        target.getBuffer(), target.length()));
}

UCollationResult RuleBasedCollator::compare(
                                             const UnicodeString& source,
                                             const UnicodeString& target,
                                             UErrorCode &status) const
{
    if(U_SUCCESS(status)) {
        return ucol_strcoll(ucollator, source.getBuffer(), source.length(),
                                       target.getBuffer(), target.length());
    } else {
        return UCOL_EQUAL;
    }
}

UCollationResult RuleBasedCollator::compare(UCharIterator &sIter,
                                            UCharIterator &tIter,
                                            UErrorCode &status) const {
    if(U_SUCCESS(status)) {
        return ucol_strcollIter(ucollator, &sIter, &tIter, &status);
    } else {
        return UCOL_EQUAL;
    }
}

CollationKey& RuleBasedCollator::getCollationKey(
                                                  const UnicodeString& source,
                                                  CollationKey& sortkey,
                                                  UErrorCode& status) const
{
    return getCollationKey(source.getBuffer(), source.length(), sortkey, status);
}

CollationKey& RuleBasedCollator::getCollationKey(const UChar* source,
                                                    int32_t sourceLen,
                                                    CollationKey& sortkey,
                                                    UErrorCode& status) const
{
    if (U_FAILURE(status))
    {
        return sortkey.setToBogus();
    }

    if ((!source) || (sourceLen == 0)) {
        return sortkey.reset();
    }

    uint8_t *result;
    int32_t resultLen = ucol_getSortKeyWithAllocation(ucollator,
                                                      source, sourceLen,
                                                      &result,
                                                      &status);
    sortkey.adopt(result, resultLen);
    return sortkey;
}

int32_t RuleBasedCollator::getMaxExpansion(int32_t order) const
{
    uint8_t result;
    UCOL_GETMAXEXPANSION(ucollator, (uint32_t)order, result);
    return result;
}

uint8_t* RuleBasedCollator::cloneRuleData(int32_t &length,
                                              UErrorCode &status)
{
    return ucol_cloneRuleData(ucollator, &length, &status);
}


int32_t RuleBasedCollator::cloneBinary(uint8_t *buffer, int32_t capacity, UErrorCode &status)
{
  return ucol_cloneBinary(ucollator, buffer, capacity, &status);
}

void RuleBasedCollator::setAttribute(UColAttribute attr,
                                     UColAttributeValue value,
                                     UErrorCode &status)
{
    if (U_FAILURE(status))
        return;
    checkOwned();
    ucol_setAttribute(ucollator, attr, value, &status);
}

UColAttributeValue RuleBasedCollator::getAttribute(UColAttribute attr,
                                                      UErrorCode &status)
{
    if (U_FAILURE(status))
        return UCOL_DEFAULT;
    return ucol_getAttribute(ucollator, attr, &status);
}

uint32_t RuleBasedCollator::setVariableTop(const UChar *varTop, int32_t len, UErrorCode &status) {
    checkOwned();
    return ucol_setVariableTop(ucollator, varTop, len, &status);
}

uint32_t RuleBasedCollator::setVariableTop(const UnicodeString varTop, UErrorCode &status) {
    checkOwned();
    return ucol_setVariableTop(ucollator, varTop.getBuffer(), varTop.length(), &status);
}

void RuleBasedCollator::setVariableTop(const uint32_t varTop, UErrorCode &status) {
    checkOwned();
    ucol_restoreVariableTop(ucollator, varTop, &status);
}

uint32_t RuleBasedCollator::getVariableTop(UErrorCode &status) const {
  return ucol_getVariableTop(ucollator, &status);
}

Collator* RuleBasedCollator::safeClone(void)
{
    UErrorCode intStatus = U_ZERO_ERROR;
    int32_t buffersize = U_COL_SAFECLONE_BUFFERSIZE;
    UCollator *ucol = ucol_safeClone(ucollator, NULL, &buffersize,
                                    &intStatus);
    if (U_FAILURE(intStatus)) {
        return NULL;
    }

    RuleBasedCollator *result = new RuleBasedCollator();
    // Null pointer check
    if (result != NULL) {
	    result->ucollator = ucol;
	    result->dataIsOwned = TRUE;
	    result->isWriteThroughAlias = FALSE;
	    setRuleStringFromCollator();
    }

    return result;
}


int32_t RuleBasedCollator::getSortKey(const UnicodeString& source,
                                         uint8_t *result, int32_t resultLength)
                                         const
{
    return ucol_getSortKey(ucollator, source.getBuffer(), source.length(), result, resultLength);
}

int32_t RuleBasedCollator::getSortKey(const UChar *source,
                                         int32_t sourceLength, uint8_t *result,
                                         int32_t resultLength) const
{
    return ucol_getSortKey(ucollator, source, sourceLength, result, resultLength);
}

Collator::ECollationStrength RuleBasedCollator::getStrength(void) const
{
    UErrorCode intStatus = U_ZERO_ERROR;
    return getECollationStrength(ucol_getAttribute(ucollator, UCOL_STRENGTH,
                                &intStatus));
}

void RuleBasedCollator::setStrength(ECollationStrength newStrength)
{
    checkOwned();
    UErrorCode intStatus = U_ZERO_ERROR;
    UCollationStrength strength = getUCollationStrength(newStrength);
    ucol_setAttribute(ucollator, UCOL_STRENGTH, strength, &intStatus);
}

int32_t RuleBasedCollator::hashCode() const
{
    int32_t length;
    const UChar *rules = ucol_getRules(ucollator, &length);
    return uhash_hashUCharsN(rules, length);
}

const Locale RuleBasedCollator::getLocale(ULocDataLocaleType type, UErrorCode &status) const {
    const char *result = ucol_getLocaleByType(ucollator, type, &status);
    if(result == NULL) {
        Locale res("");
        res.setToBogus();
        return res;
    } else {
        return Locale(result);
    }
}

void
RuleBasedCollator::setLocales(const Locale& requestedLocale, const Locale& validLocale, const Locale& actualLocale) {
    checkOwned();
    char* rloc  = uprv_strdup(requestedLocale.getName());
    if (rloc) {
        char* vloc = uprv_strdup(validLocale.getName());
        if (vloc) {
            char* aloc = uprv_strdup(actualLocale.getName());
            if (aloc) {
                ucol_setReqValidLocales(ucollator, rloc, vloc, aloc);
                return;
            }
            uprv_free(vloc);
        }
        uprv_free(rloc);
    }
}

// RuleBaseCollatorNew private constructor ----------------------------------

RuleBasedCollator::RuleBasedCollator()
  : dataIsOwned(FALSE), isWriteThroughAlias(FALSE), ucollator(NULL)
{
}

RuleBasedCollator::RuleBasedCollator(const Locale& desiredLocale,
                                           UErrorCode& status)
 : dataIsOwned(FALSE), isWriteThroughAlias(FALSE), ucollator(NULL)
{
    if (U_FAILURE(status))
        return;

    /*
    Try to load, in order:
     1. The desired locale's collation.
     2. A fallback of the desired locale.
     3. The default locale's collation.
     4. A fallback of the default locale.
     5. The default collation rules, which contains en_US collation rules.

     To reiterate, we try:
     Specific:
      language+country+variant
      language+country
      language
     Default:
      language+country+variant
      language+country
      language
     Root: (aka DEFAULTRULES)
     steps 1-5 are handled by resource bundle fallback mechanism.
     however, in a very unprobable situation that no resource bundle
     data exists, step 5 is repeated with hardcoded default rules.
    */

    setUCollator(desiredLocale, status);

    if (U_FAILURE(status))
    {
        status = U_ZERO_ERROR;

        setUCollator(kRootLocaleName, status);
        if (status == U_ZERO_ERROR) {
            status = U_USING_DEFAULT_WARNING;
        }
    }

    if (U_SUCCESS(status))
    {
        setRuleStringFromCollator();
    }
}

void
RuleBasedCollator::setUCollator(const char *locale,
                                UErrorCode &status)
{
    if (U_FAILURE(status))
        return;
    if (ucollator && dataIsOwned)
        ucol_close(ucollator);
    ucollator = ucol_open_internal(locale, &status);
    dataIsOwned = TRUE;
    isWriteThroughAlias = FALSE;
}


void
RuleBasedCollator::checkOwned() {
    if (!(dataIsOwned || isWriteThroughAlias)) {
        UErrorCode status = U_ZERO_ERROR;
        ucollator = ucol_safeClone(ucollator, NULL, NULL, &status);
        setRuleStringFromCollator();
        dataIsOwned = TRUE;
        isWriteThroughAlias = FALSE;
    }
}

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(RuleBasedCollator)

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_COLLATION */
