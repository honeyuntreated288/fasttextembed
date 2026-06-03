{
  "targets": [
    {
      "target_name": "fte_native",
      "sources": [
        "addon.c",
        "../csrc/src/api.c",
        "../csrc/src/loader.c",
        "../csrc/src/model_bert.c",
        "../csrc/src/pack.c",
        "../csrc/src/tensor.c",
        "../csrc/src/threadpool.c",
        "../csrc/src/layer_matmul_names.c",
        "../csrc/src/kernels/kernels_scalar.c",
        "../csrc/src/tokenizer/tokenizer.c"
      ],
      "include_dirs": ["../csrc/include", "../csrc/src"],
      "libraries": ["-lm"],
      "cflags_c": ["-O3", "-std=c11"],
      "conditions": [
        ["target_arch=='arm64'", {
          "cflags_c": ["-mcpu=native"],
          "xcode_settings": { "OTHER_CFLAGS": ["-O3", "-std=c11", "-mcpu=native"] }
        }],
        ["target_arch=='x64'", {
          "cflags_c": ["-march=native"],
          "xcode_settings": { "OTHER_CFLAGS": ["-O3", "-std=c11", "-march=native"] }
        }]
      ]
    }
  ]
}
