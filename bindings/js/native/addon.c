/* Node N-API binding to the C engine (max-speed, Node-only). The portable WASM backend
 * is the fallback when this native addon isn't built. */
#include <node_api.h>
#include <stdlib.h>
#include <string.h>
#include "fte/fte.h"

#define DIM 384

static fte_model *handle_of(napi_env env, napi_value v) {
    void *p = NULL;
    napi_get_value_external(env, v, &p);
    return (fte_model *)p;
}

/* init(modelPath, vocabPath) -> external handle */
static napi_value Init(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    char path[2048], vocab[2048];
    size_t n;
    napi_get_value_string_utf8(env, args[0], path, sizeof path, &n);
    napi_get_value_string_utf8(env, args[1], vocab, sizeof vocab, &n);
    fte_model *m = NULL;
    if (fte_init(path, vocab, &m) != FTE_OK) {
        napi_throw_error(env, NULL, "fte_init failed");
        return NULL;
    }
    napi_value ext;
    napi_create_external(env, m, NULL, NULL, &ext);
    return ext;
}

static napi_value f32array(napi_env env, const float *src, size_t dim) {
    napi_value buf, ta;
    void *data;
    napi_create_arraybuffer(env, dim * sizeof(float), &data, &buf);
    if (src) memcpy(data, src, dim * sizeof(float));
    napi_create_typedarray(env, napi_float32_array, dim, buf, 0, &ta);
    return ta;
}

/* embedOne(handle, text) -> Float32Array(384) */
static napi_value EmbedOne(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    fte_model *m = handle_of(env, args[0]);
    size_t len;
    napi_get_value_string_utf8(env, args[1], NULL, 0, &len);
    char *txt = malloc(len + 1);
    size_t n;
    napi_get_value_string_utf8(env, args[1], txt, len + 1, &n);
    float out[DIM];
    fte_embed(m, txt, out);
    free(txt);
    return f32array(env, out, DIM);
}

/* embedBatch(handle, [text,...]) -> [Float32Array(384), ...] */
static napi_value EmbedBatch(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    fte_model *m = handle_of(env, args[0]);
    uint32_t count;
    napi_get_array_length(env, args[1], &count);
    char **texts = malloc(count * sizeof(char *));
    for (uint32_t i = 0; i < count; i++) {
        napi_value s;
        napi_get_element(env, args[1], i, &s);
        size_t len;
        napi_get_value_string_utf8(env, s, NULL, 0, &len);
        texts[i] = malloc(len + 1);
        size_t n;
        napi_get_value_string_utf8(env, s, texts[i], len + 1, &n);
    }
    float *out = malloc((size_t)count * DIM * sizeof(float));
    fte_embed_batch(m, (const char *const *)texts, count, out, 0);
    napi_value result;
    napi_create_array_with_length(env, count, &result);
    for (uint32_t i = 0; i < count; i++) {
        napi_set_element(env, result, i, f32array(env, out + (size_t)i * DIM, DIM));
        free(texts[i]);
    }
    free(texts);
    free(out);
    return result;
}

static napi_value Free(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    fte_free(handle_of(env, args[0]));
    return NULL;
}

static napi_value Register(napi_env env, napi_value exports) {
    napi_value fn;
    napi_create_function(env, NULL, 0, Init, NULL, &fn);
    napi_set_named_property(env, exports, "init", fn);
    napi_create_function(env, NULL, 0, EmbedOne, NULL, &fn);
    napi_set_named_property(env, exports, "embedOne", fn);
    napi_create_function(env, NULL, 0, EmbedBatch, NULL, &fn);
    napi_set_named_property(env, exports, "embedBatch", fn);
    napi_create_function(env, NULL, 0, Free, NULL, &fn);
    napi_set_named_property(env, exports, "free", fn);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Register)
