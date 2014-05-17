
#ifndef UDEPRCTD_H
#define UDEPRCTD_H

#ifdef U_HIDE_DEPRECATED_API

#    if U_DISABLE_RENAMING
#        define ucol_getContractions ucol_getContractions_DEPRECATED_API_DO_NOT_USE
#        define ucol_getLocale ucol_getLocale_DEPRECATED_API_DO_NOT_USE
#        define ures_countArrayItems ures_countArrayItems_DEPRECATED_API_DO_NOT_USE
#        define ures_getLocale ures_getLocale_DEPRECATED_API_DO_NOT_USE
#        define ures_getVersionNumber ures_getVersionNumber_DEPRECATED_API_DO_NOT_USE
#        define utrans_getAvailableID utrans_getAvailableID_DEPRECATED_API_DO_NOT_USE
#        define utrans_getID utrans_getID_DEPRECATED_API_DO_NOT_USE
#        define utrans_open utrans_open_DEPRECATED_API_DO_NOT_USE
#        define utrans_unregister utrans_unregister_DEPRECATED_API_DO_NOT_USE
#    else
#        define ucol_getContractions_4_4 ucol_getContractions_DEPRECATED_API_DO_NOT_USE
#        define ucol_getLocale_4_4 ucol_getLocale_DEPRECATED_API_DO_NOT_USE
#        define ures_countArrayItems_4_4 ures_countArrayItems_DEPRECATED_API_DO_NOT_USE
#        define ures_getLocale_4_4 ures_getLocale_DEPRECATED_API_DO_NOT_USE
#        define ures_getVersionNumber_4_4 ures_getVersionNumber_DEPRECATED_API_DO_NOT_USE
#        define utrans_getAvailableID_4_4 utrans_getAvailableID_DEPRECATED_API_DO_NOT_USE
#        define utrans_getID_4_4 utrans_getID_DEPRECATED_API_DO_NOT_USE
#        define utrans_open_4_4 utrans_open_DEPRECATED_API_DO_NOT_USE
#        define utrans_unregister_4_4 utrans_unregister_DEPRECATED_API_DO_NOT_USE
#    endif /* U_DISABLE_RENAMING */

#endif /* U_HIDE_DEPRECATED_API */
#endif /* UDEPRCTD_H */

