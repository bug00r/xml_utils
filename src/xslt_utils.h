#ifndef XSLT_UTILS_H
#define XSLT_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <libxml/hash.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/extensions.h>
#include <libxslt/variables.h>
#include <libxslt/documents.h>
#include <libxslt/xsltutils.h>

#include "string_utils.h"
#include "xml_utils.h"
#include "dl_list.h"

typedef struct {
    XmlCtx           *xml;           //required
    xsltStylesheetPtr   stylesheet;     //required (automatic cleaned if exist)
    const char          **text_params;  //optional (NULL)
    const char          **xpath_params; //optional (NULL)
    const char          * output;       //optional (NULL)
    FILE                *profile;       //optional (NULL)
    DlList           *errors;        //automatic usage
} XsltCtx;


void xslt_ctx_init(XsltCtx *ctx);
void xslt_ctx_cleanup(XsltCtx *ctx);
xmlDocPtr do_xslt(XsltCtx * ctx);
void xslt_print_err(XsltCtx * ctx);

#endif