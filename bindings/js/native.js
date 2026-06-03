// Native N-API-backed TextEmbedding (Node-only, max speed). Requires the compiled
// addon (`npm run build:native`); index.js falls back to wasm.js when it's absent.
"use strict";
const path = require("path");

// Throws if the addon isn't built — index.js catches this and falls back to WASM.
const addon = require("./native/build/Release/fte_native.node");

const DIM = 384;

class TextEmbedding {
  constructor(handle) {
    this._h = handle;
  }

  // opts: { fte: <path>, vocab: <path> }. In Node you can omit and it resolves/downloads
  // via ./model.js (the native addon loads from files, not memory).
  static async create(opts = {}) {
    let { fte, vocab } = opts;
    if (!fte || !vocab) {
      const m = require("./model.js");
      ({ fte, vocab } = await m.resolveModelPaths());
    }
    return new TextEmbedding(addon.init(path.resolve(fte), path.resolve(vocab)));
  }

  embedOne(text) {
    return Array.from(addon.embedOne(this._h, text));
  }

  embed(texts) {
    if (texts.length === 0) return [];
    return addon.embedBatch(this._h, texts).map((a) => Array.from(a));
  }

  free() {
    if (this._h) { addon.free(this._h); this._h = null; }
  }
}

module.exports = { TextEmbedding, DIM };
