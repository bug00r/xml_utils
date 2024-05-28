#include "xml_utils.h"

static XmlCtx* __xml_ctx_create(const XmlSource *xml_src, xmlDocPtr doc) {
    XmlCtx temp = {xml_src, doc, {XML_CTX_SUCCESS, XML_CTX_NO_REASON}};
    XmlCtx * new_ctx = malloc(sizeof(XmlCtx));
    memcpy(new_ctx, &temp, sizeof(XmlCtx));
    return new_ctx;
}

static void __xml_ctx_set_state(XmlCtx * ctx,  XmlCtxStateNo state_no, XmlCtxStateReason  reason ) {
    ctx->state.state_no = state_no;
    ctx->state.reason   = reason;
}

static void __xml_ctx_set_state_ptr(XmlCtx * ctx,  XmlCtxStateNo *state_no, XmlCtxStateReason *reason ) {
    __xml_ctx_set_state(ctx, *state_no, *reason);
}

static bool __xml_ctx_valid( XmlCtx *ctx ) {
    bool isvalid = true;

    if ( ctx == NULL || ctx->doc == NULL) {
    
        __xml_ctx_set_state(ctx, XML_CTX_ERROR, XML_CTX_SRC_INVALID);
        isvalid = false;
    
    } else {
    
        __xml_ctx_set_state(ctx, XML_CTX_SUCCESS, XML_CTX_NO_REASON);
    
    }
    return isvalid;
}

static bool __xml_ctx_xpath_valid( XmlCtx *ctx, const char *xpath) {
    bool isvalid = true;

    if ( xpath == NULL || (strlen(xpath) == 0) ) {
        __xml_ctx_set_state(ctx, XML_CTX_ERROR, XML_CTX_XPATH_INVALID);
        isvalid = false;
    
    } else {
    
        __xml_ctx_set_state(ctx, XML_CTX_SUCCESS, XML_CTX_NO_REASON);
    
    }
    return isvalid;
}

static void __xml_ctx_attr_str_xpptr(xmlXPathObjectPtr node, const unsigned char *value) {

    if ( xml_xpath_has_result(node) ) {

        const int maxNodes = node->nodesetval->nodeNr;

        xmlNodePtr *nodes = node->nodesetval->nodeTab;
        
        for (int curattr = 0;curattr < maxNodes ; ++curattr) {
            
            xmlNodePtr node = nodes[curattr];
            
            if (node->type == XML_ATTRIBUTE_NODE) {

                xmlSetProp(node->parent, node->name, (xmlChar*)value);
            
            }
        
        }
    }
}

static void __xml_ctx_content_xpptr(xmlXPathObjectPtr node, const unsigned char *value) {

    if ( xml_xpath_has_result(node) ) {
        
        const int maxNodes = node->nodesetval->nodeNr;

        xmlNodePtr *nodes = node->nodesetval->nodeTab;
        
        for (int curattr = 0;curattr < maxNodes ; ++curattr) {
            
            xmlNodePtr node = nodes[curattr];

            xmlNodePtr parent = node->parent;

            if (node->type == XML_CDATA_SECTION_NODE ) { 
                
                xmlUnlinkNode(node);
                xmlFreeNode(node);

                xmlNodePtr content_node = xmlNewCDataBlock(parent->doc, BAD_CAST value, strlen((const char *)value));
                xmlAddChild(parent, content_node);
            }
        
        }
    }

}

#if 0
//
// EOF private section
//
#endif


XmlCtx* xml_ctx_new_empty() {

    xmlDocPtr doc = xmlNewDoc((xmlChar *)"1.0");

    XmlCtx *new_ctx = __xml_ctx_create(NULL, doc);
    __xml_ctx_set_state(new_ctx, XML_CTX_SUCCESS, XML_CTX_READ_AND_PARSE);

    return new_ctx;
}

XmlCtx* xml_ctx_new_empty_root_name(const char* rootname) {
    XmlCtx* new_ctx = xml_ctx_new_empty();

    if(rootname && strlen(rootname) > 0) {
        xmlNodePtr newroot = xmlNewNode(NULL, (xmlChar *) rootname);
        xmlDocSetRootElement(new_ctx->doc, newroot);
    }

    return new_ctx;
}


XmlCtx* xml_ctx_new(const XmlSource *xml_src) {

    xmlDocPtr doc = NULL;

    XmlCtxStateNo state_no = XML_CTX_SUCCESS; 
    XmlCtxStateReason reason = XML_CTX_READ_AND_PARSE;

    if ( xml_src != NULL && xml_src->src_data != NULL && *xml_src->src_size > 0 ) {

        doc = xmlReadMemory((const char *)xml_src->src_data, *xml_src->src_size, "noname.xml", NULL, 0);
        
    } else {
        state_no = XML_CTX_ERROR; 
    }
    
    if ( xmlGetLastError() != NULL ) {
        state_no = XML_CTX_ERROR; 
        xmlFreeDoc(doc);
        doc = NULL;
    }

    XmlCtx *new_ctx = __xml_ctx_create(xml_src, doc);
    __xml_ctx_set_state_ptr(new_ctx, &state_no, &reason);

    return new_ctx;
}

XmlCtx* xml_ctx_new_node(const xmlNodePtr rootnode) {
    XmlCtx *new_ctx = xml_ctx_new_empty();
    xmlNodePtr copyroot = xmlCopyNode(rootnode, 1);
    xmlDocSetRootElement(new_ctx->doc ,copyroot);
    return new_ctx;
}

XmlCtx* xml_ctx_new_file(const char *filename) 
{
    XmlCtx *new_ctx = xml_ctx_new_empty();
    
    XmlCtxStateNo state_no = XML_CTX_SUCCESS; 
    XmlCtxStateReason reason = XML_CTX_READ_AND_PARSE;

    if (u_file_exists(filename) && (xmlGetLastError() == NULL))
    {
        new_ctx->doc = xmlReadFile(filename, "UTF-8", 0);
    }
    else 
    {
        state_no = XML_CTX_ERROR;
        xmlFreeDoc(new_ctx->doc);
        new_ctx->doc = NULL;
    }
    
    __xml_ctx_set_state_ptr(new_ctx, &state_no, &reason);

    return new_ctx;
}

void xml_ctx_save_file(const XmlCtx *ctx, const char *filename) {

    XmlCtxStateNo state_no = XML_CTX_SUCCESS; 
    XmlCtxStateReason reason = XML_CTX_READ_AND_PARSE; 

    if (ctx != NULL && ctx->doc != NULL && filename != NULL && ( strlen(filename) > 0 )) {

        if ( xmlSaveFileEnc(filename, ctx->doc, "UTF-8") == -1 ) {
            
            xmlErrorPtr	err = xmlGetLastError();

            if (err != NULL) {
                state_no = XML_CTX_ERROR;
                printf("XML save error: %s: %s\n%s\n%s\n", err->message, err->str1,err->str2, err->str1);
            }
            
        }

    }

    __xml_ctx_set_state_ptr((XmlCtx *)ctx, &state_no, &reason);

}

void free_xml_ctx(XmlCtx **ctx) {
    
    if ( ctx != NULL && *ctx != NULL ) {
        XmlCtx *todelete_ctx = *ctx;
        
        if (todelete_ctx->doc) {
            xmlDocGetRootElement(todelete_ctx->doc);
            xmlFreeDoc(todelete_ctx->doc);
        }

        free(todelete_ctx);
        *ctx = NULL;
    }
}
void free_xml_ctx_ptr(XmlCtx *ctx) {
    if ( ctx != NULL ) {
        XmlCtx *todelete_ctx = ctx;
        
        if (todelete_ctx->doc) {
            xmlDocGetRootElement(todelete_ctx->doc);
            xmlFreeDoc(todelete_ctx->doc);
        }

        free(todelete_ctx);
    }
}

void free_xml_ctx_src(XmlCtx **ctx) {
    if ( ctx != NULL && *ctx != NULL ) {
        XmlCtx *todelete_ctx = *ctx;

        XmlSource * _src = (XmlSource*)todelete_ctx->src;

        free_xml_ctx(ctx);

        if(_src) {
            xml_source_free(&_src);
        }
    }
}

xmlXPathObjectPtr xml_ctx_xpath( const XmlCtx *ctx, const char *xpath) {

    xmlXPathObjectPtr result = NULL;

    if(ctx->doc && xpath) {
        
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(ctx->doc);
        xmlXPathRegisterAllFunctions(xpathCtx);
        xmlXPathRegisterFunc(xpathCtx,(const xmlChar *) "regexmatch", regexmatch_xpath_func);
        xmlXPathRegisterFunc(xpathCtx,(const xmlChar *) "max", max_xpath_func);
        xmlXPathRegisterFunc(xpathCtx,(const xmlChar *) "in_range", str_in_range_xpath_func); 

        if ( xpathCtx != NULL ) {
            
            result = xmlXPathEvalExpression((const xmlChar*)xpath, xpathCtx);
                            
        }
        
        xmlXPathFreeContext(xpathCtx);
    }

    return result;
}


xmlXPathObjectPtr xml_ctx_xpath_format( const XmlCtx *ctx, const char *xpath_format, ...) {
    
    va_list args;
    va_start(args, xpath_format);
    char *gen_xpath = format_string_va_new(xpath_format, args);
    va_end(args);

    #if debug > 0
        printf("gen xpath: %s\n", gen_xpath);
    #endif

    xmlXPathObjectPtr result = xml_ctx_xpath(ctx, gen_xpath);

    free(gen_xpath);
    
    return result;
}

xmlXPathObjectPtr xml_ctx_xpath_format_va( const XmlCtx *ctx, const char *xpath_format, va_list argptr) {
    
    char *gen_xpath = format_string_va_new(xpath_format, argptr);
    
    #if debug > 0
        printf("gen xpath: %s\n", gen_xpath);
    #endif

    xmlXPathObjectPtr result = xml_ctx_xpath(ctx, gen_xpath);

    free(gen_xpath);
    
    return result;
}

void xml_ctx_nodes_add_xpath(XmlCtx *src, const char *src_xpath, XmlCtx *dst, const char *dst_xpath) {
    
    if ( !__xml_ctx_valid(src) || !__xml_ctx_valid(dst) ) return;
    
    if ( !__xml_ctx_xpath_valid(src, src_xpath) || !__xml_ctx_xpath_valid(dst, dst_xpath) ) return;

    xmlXPathObjectPtr srcxpres = xml_ctx_xpath(src, src_xpath);

    if ( xml_xpath_has_result(srcxpres) ) {

        const int numsrcs = srcxpres->nodesetval->nodeNr;
        xmlNodePtr * sources = srcxpres->nodesetval->nodeTab;

        xmlXPathObjectPtr dstxpres = xml_ctx_xpath(dst, dst_xpath);

        if ( xml_xpath_has_result(dstxpres) ) {

            for(int cursrcnum = 0; cursrcnum < numsrcs; ++cursrcnum) {
                
                #if debug > 1
                    printf("source: %i of %i\n",cursrcnum , numsrcs);
                #endif

                xmlNodePtr cursrc = sources[cursrcnum];

                const int numtargets = dstxpres->nodesetval->nodeNr;
                xmlNodePtr * targets = dstxpres->nodesetval->nodeTab;

                for(int curtargetnum = 0; curtargetnum < numtargets; ++curtargetnum) {
                    
                    xmlNodePtr curtarget = targets[curtargetnum];

                    xmlNodePtr result = NULL;
                    xmlNodePtr copy = NULL;

                    #if debug > 1
                            printf("target: %i of %i\n",curtargetnum , numtargets);
                    #endif

                    if ( numtargets == 1 ) {
                        
                        #if debug > 1
                            printf("target is node!!! \n");
                        #endif

                        copy = xmlCopyNode(cursrc, 1);
                        result = xmlAddChild(curtarget, copy);
                    } else {

                        #if debug > 1
                            printf("target is list!!! \n");
                        #endif

                        copy = xmlCopyNodeList(cursrc);
                        result = xmlAddChildList(curtarget, copy);
                    }
                    
                    if(result == NULL) {
                        __xml_ctx_set_state(dst, XML_CTX_ERROR, XML_CTX_ADD);
                    } else {
                        __xml_ctx_set_state(dst, XML_CTX_SUCCESS, XML_CTX_ADD);
                    }

                }
            }
        }

        xmlXPathFreeObject(dstxpres);
    }
    xmlXPathFreeObject(srcxpres);
}

void xml_ctx_nodes_add_note_xpres(xmlNodePtr src_node, xmlXPathObjectPtr dst_result) {
    
    if ( xml_xpath_has_result(dst_result) ) {

        const int maxNodes = dst_result->nodesetval->nodeNr;
        xmlNodePtr *nodes = dst_result->nodesetval->nodeTab;

        for(int curNode = 0; curNode < maxNodes; ++curNode) {

            xmlNodePtr target_node = nodes[curNode];
            
            xmlNodePtr copy = xmlCopyNode(src_node, 1);
            
            xmlAddChild(target_node, copy);
        }

    }

}

void xml_ctx_nodes_add_node_xpath(xmlNodePtr src_node, XmlCtx *dst, const char *dst_xpath) {

    xmlXPathObjectPtr target_node_result = xml_ctx_xpath(dst, dst_xpath);

    xml_ctx_nodes_add_note_xpres(src_node, target_node_result);

    xmlXPathFreeObject(target_node_result);
}

void xml_ctx_nodes_add_node_xpath_format(xmlNodePtr src_node, XmlCtx *dst, const char *dst_xpath, ...) {

    va_list args;
    va_start(args, dst_xpath);
    xmlXPathObjectPtr target_node_result = xml_ctx_xpath_format_va(dst, dst_xpath, args);
    va_end(args);

    xml_ctx_nodes_add_note_xpres(src_node, target_node_result);

    xmlXPathFreeObject(target_node_result);
}

void xml_ctx_rem_nodes_xpres(xmlXPathObjectPtr xpres) {
    
    if (xml_xpath_has_result(xpres)) {
        const int cntNodes = xpres->nodesetval->nodeNr;
        for(int curnode = 0; curnode < cntNodes; ++curnode) {
            xmlNodePtr cn = xpres->nodesetval->nodeTab[curnode];
            xmlUnlinkNode(cn);
            xmlFreeNode(cn);
        }
    }

}


void xml_ctx_remove(XmlCtx *ctx, const char *xpath) {

    xmlXPathObjectPtr found = xml_ctx_xpath_format(ctx, xpath);

    xml_ctx_rem_nodes_xpres(found);

    xmlXPathFreeObject(found);
}

void xml_ctx_remove_format(XmlCtx *ctx, const char *xpath_format, ...) {

    va_list args;
    va_start(args, xpath_format);
    xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);
    va_end(args);

    xml_ctx_rem_nodes_xpres(found);

    xmlXPathFreeObject(found);
}

bool xml_ctx_exist(XmlCtx *ctx, const char *xpath) {
    xmlXPathObjectPtr found = xml_ctx_xpath(ctx, xpath);

    bool exist = xml_xpath_has_result(found);

    xmlXPathFreeObject(found);

    return exist;
}

bool xml_ctx_exist_format(XmlCtx *ctx, const char *xpath_format, ...) {

    va_list args;
    va_start(args, xpath_format);
    xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);
    va_end(args);

    bool exist = xml_xpath_has_result(found);

    xmlXPathFreeObject(found);

    return exist;
    
}

bool xml_xpath_has_result(xmlXPathObjectPtr xpathobj) {
    return ( xpathobj != NULL && xpathobj->type == XPATH_NODESET && xpathobj->nodesetval && (xpathobj->nodesetval->nodeNr > 0 ));
}

void xml_ctx_set_attr_str_xpath(XmlCtx *ctx, const unsigned char *value, const char *xpath) {
    
    xmlXPathObjectPtr found = xml_ctx_xpath(ctx, xpath);

    __xml_ctx_attr_str_xpptr(found, value);

    xmlXPathFreeObject(found);

}

void xml_ctx_set_attr_str_xpath_format(XmlCtx *ctx, const unsigned char *value, const char *xpath_format, ...) {
    
    va_list args;
    va_start(args, xpath_format);

    xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);

    __xml_ctx_attr_str_xpptr(found, value);

    va_end(args);

    xmlXPathFreeObject(found);
}

void xml_ctx_set_content_xpath(XmlCtx *ctx, const unsigned char *value, const char *xpath) {
    xmlXPathObjectPtr found = xml_ctx_xpath(ctx, xpath);

    __xml_ctx_content_xpptr(found, value);

    xmlXPathFreeObject(found);
}

void xml_ctx_set_content_xpath_format(XmlCtx *ctx, const unsigned char *value, const char *xpath_format, ...) {
    va_list args;
    va_start(args, xpath_format);

    xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);

    __xml_ctx_content_xpptr(found, value);

    va_end(args);

    xmlXPathFreeObject(found);
}

xmlChar * xml_ctx_get_attr(XmlCtx *ctx, const unsigned char *attr_name, const char *xpath) {
    
    xmlChar *value = NULL;
    
    if (ctx != NULL) {

        xmlXPathObjectPtr found = xml_ctx_xpath(ctx, xpath);

        if (xml_xpath_has_result(found)) {
            value = xmlGetProp(found->nodesetval->nodeTab[0], (const xmlChar*)attr_name);
        }

        xmlXPathFreeObject(found);

    }

    return value;

}

xmlChar * xml_ctx_get_attr_format(XmlCtx *ctx, const unsigned char *attr_name, const char *xpath_format, ...) {
    
    xmlChar *value = NULL;
    
    if (ctx != NULL) {

        va_list args;
        va_start(args, xpath_format);

        xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);

        va_end(args);

        if (xml_xpath_has_result(found)) {
            value = xmlGetProp(found->nodesetval->nodeTab[0], (const xmlChar*)attr_name);
        }

        xmlXPathFreeObject(found);
    
    }

    return value;
}

//The FOLLOWING IS UNTESTED AND INCOMPLETE

int xml_ctx_strtol(xmlChar *str, long *result)
{
    int parseError = 0;
    char *end;

    *result = strtol((const char*)str, &end, 10);
    
    if (errno == ERANGE){
        errno = 0;
        parseError = 1;
    }
    
    return parseError;
}

int xml_ctx_strtof(xmlChar *str, float *result)
{
    int parseError = 0;
    char *end;

    *result = strtof((const char*)str, &end);
    
    if (errno == ERANGE){
        errno = 0;
        parseError = 1;
    }
    
    return parseError;
}

int xml_ctx_xpath_tod(XmlCtx *ctx, double *result, const char *xpath)
{
    int errNo = 1;
    if ( ctx != NULL )
    {
        xmlXPathObjectPtr found = xml_ctx_xpath(ctx, xpath);

        if ( found && ( xml_xpath_has_result(found) || found->type == XPATH_NUMBER))
        {
            *result = xmlXPathCastToNumber(found);
            errNo = ( isnan(*result) ? 1 : 0);
        }

        xmlXPathFreeObject(found);
    } 

    return errNo;
}

int xml_ctx_xpath_tod_format_va(XmlCtx *ctx, double *result, const char *xpath_format, va_list args)
{
    int errNo = 1;
    if ( ctx != NULL )
    {
        xmlXPathObjectPtr found = xml_ctx_xpath_format_va(ctx, xpath_format, args);

        if ( found && ( xml_xpath_has_result(found) || found->type == XPATH_NUMBER))
        {
            *result = xmlXPathCastToNumber(found);
            errNo = ( isnan(*result) ? 1 : 0);
        }

        xmlXPathFreeObject(found);
    } 

    return errNo;
}

int xml_ctx_xpath_tol(XmlCtx *ctx, long *result, const char *xpath)
{
    double dResult;
    int errNo = xml_ctx_xpath_tod(ctx, &dResult, xpath);

    if ( errNo == 0 )
    {
        *result = (long)dResult;
    }

    return errNo;
}

int xml_ctx_xpath_tol_format(XmlCtx *ctx, long *result, const char *xpath_format, ...)
{
    double dResult;

    va_list args;
    va_start(args, xpath_format);

    int errNo = xml_ctx_xpath_tod_format_va(ctx, &dResult, xpath_format, args);

    va_end(args);

    if ( errNo == 0 )
    {
        *result = (long)dResult;
    }

    return errNo;
}

int xml_ctx_xpath_tof(XmlCtx *ctx, float *result, const char *xpath)
{
    double dResult;
    int errNo = xml_ctx_xpath_tod(ctx, &dResult, xpath);

    if ( errNo == 0 )
    {
        *result = (long)dResult;
    }

    return errNo;
}

int xml_ctx_xpath_tof_format(XmlCtx *ctx, float *result, const char *xpath_format, ...)
{
    double dResult;

    va_list args;
    va_start(args, xpath_format);

    int errNo = xml_ctx_xpath_tod_format_va(ctx, &dResult, xpath_format, args);

    va_end(args);
    
    if ( errNo == 0 )
    {
        *result = (long)dResult;
    }

    return errNo;
}



