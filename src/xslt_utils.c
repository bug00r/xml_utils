#include "xslt_utils.h"

static void /*LIBXSLT_ATTR_FORMAT(2,3)*/
_xslt_add_error_to_list(void *ctx, const char *msg, ...) {

	va_list vl;
	va_start(vl, msg);
    dl_list_append( (DlList*)ctx, format_string_va_new(msg, vl));
	va_end(vl);

}

static void
_xslt_cleanup_error(void **data) {
	if (data && *data) {
		free(*data);
		*data = NULL;
	}
}

static void
_xslt_print_err(void **data) {
	if (data && *data) {
		printf((char*)*data);
	}
}

static void _xslt_reset(XsltCtx *ctx) {
		ctx->output			= NULL;
		ctx->text_params	= NULL;
		ctx->xpath_params	= NULL;
		ctx->profile	= NULL;
		ctx->stylesheet = NULL;
		ctx->xml		= NULL;
		ctx->errors		= NULL;
}

void xslt_ctx_init(XsltCtx *ctx) {
	if (ctx) {
		_xslt_reset(ctx);
		ctx->errors		= dl_list_new();
	}
}

void xslt_ctx_cleanup(XsltCtx *ctx) {
	if (ctx) {
		dl_list_each(ctx->errors, _xslt_cleanup_error);
		dl_list_free(&ctx->errors);

		if (ctx->stylesheet != NULL) {
			xsltFreeStylesheet(ctx->stylesheet);
		}

		_xslt_reset(ctx);
	}
}

xmlDocPtr do_xslt(XsltCtx * ctx) {
	xmlDocPtr result = NULL;

	if (ctx) {
		
		xmlDocPtr input_doc = ctx->xml->doc;

		if ( input_doc && ctx->stylesheet ) {

			xsltTransformContextPtr xslt_ctx = xsltNewTransformContext(ctx->stylesheet, input_doc);
			
			xsltSetTransformErrorFunc(xslt_ctx, ctx->errors, _xslt_add_error_to_list);

			xsltQuoteUserParams(xslt_ctx, ctx->text_params);
			xsltEvalUserParams(xslt_ctx, ctx->xpath_params);
			result = xsltApplyStylesheetUser(ctx->stylesheet, input_doc, NULL /*ctx->params */, ctx->output, ctx->profile, xslt_ctx);

			xsltFreeTransformContext(xslt_ctx);
			
			xsltCleanupGlobals();
		}

	}

	return result;
}

void xslt_print_err(XsltCtx * ctx) {
	dl_list_each(ctx->errors, _xslt_print_err);
}