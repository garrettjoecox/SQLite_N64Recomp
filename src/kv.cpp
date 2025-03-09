#include "sqlite3.h"
#include "helpers.hpp"
#include "mod_recomp.h"

#define DB_FILE "ProxyRecomp_KV.db"

sqlite3 *db;
int kvState = -1;

extern "C" {

DLLEXPORT uint32_t recomp_api_version = 1;

int KV_InitImpl() {
    if (kvState != -1) return kvState;

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        printf("[ProxyRecomp_KV] Failed init, can't open database: %s\n", sqlite3_errmsg(db));
        kvState = 0;
        return kvState;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS storage (key TEXT PRIMARY KEY, value BLOB NOT NULL);";
    kvState = sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK;
    if (!kvState) {
        printf("[ProxyRecomp_KV] Failed init, failed table creation: %s\n", sqlite3_errmsg(db));
    } else {
        printf("[ProxyRecomp_KV] Initialized");
    }

    return kvState;
}

DLLEXPORT void KV_Init(uint8_t* rdram, recomp_context* ctx) {
    _return(ctx, KV_InitImpl());
}

DLLEXPORT void KV_Teardown(uint8_t* rdram, recomp_context* ctx) {
    sqlite3_close(db);
}

DLLEXPORT void KV_Set(uint8_t* rdram, recomp_context* ctx) {
    std::string key = _arg_string<0>(rdram, ctx);
    void* data = _arg<1, void*>(rdram, ctx);
    uint32_t size = _arg<2, uint32_t>(rdram, ctx);

    if (!KV_InitImpl()) {
        printf("[ProxyRecomp_KV] Failed SET %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }

    const char *sql = "INSERT INTO storage (key, value) VALUES (?, ?) ON CONFLICT(key) DO UPDATE SET value = excluded.value;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("[ProxyRecomp_KV] Failed SET %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, data, size, SQLITE_STATIC);
    int res = sqlite3_step(stmt) == SQLITE_DONE;
    if (!res) {
        printf("[ProxyRecomp_KV] Failed SET %s: %s\n", key.c_str(), sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    _return(ctx, res);
}

DLLEXPORT void KV_Get(uint8_t* rdram, recomp_context* ctx) {
    std::string key = _arg_string<0>(rdram, ctx);
    void* dest = _arg<1, void*>(rdram, ctx);
    uint32_t expected_size = _arg<2, uint32_t>(rdram, ctx);

    if (!KV_InitImpl()) {
        printf("[ProxyRecomp_KV] Failed GET %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }

    const char *sql = "SELECT value FROM storage WHERE key = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("[ProxyRecomp_KV] Failed GET %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        size_t stored_size = sqlite3_column_bytes(stmt, 0);
        if (stored_size != expected_size) {  // Fail if sizes don't match
            printf("[ProxyRecomp_KV] Failed GET %s: Size doesn't match\n", key.c_str());
            sqlite3_finalize(stmt);
            _return(ctx, 0);
            return;
        }
        memcpy(dest, sqlite3_column_blob(stmt, 0), stored_size);
        sqlite3_finalize(stmt);
        _return(ctx, 1);
        return;
    }

    // No need to log this I don't think?
    // printf("[ProxyRecomp_KV] Failed getting %s: Nothing stored\n", key.c_str());
    sqlite3_finalize(stmt);
    _return(ctx, 0);
}

DLLEXPORT void KV_Remove(uint8_t* rdram, recomp_context* ctx) {
    std::string key = _arg_string<0>(rdram, ctx);

    if (!KV_InitImpl()) {
        printf("[ProxyRecomp_KV] Failed REMOVE %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }

    const char *sql = "DELETE FROM storage WHERE key = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("[ProxyRecomp_KV] Failed REMOVE %s: %s\n", key.c_str(), sqlite3_errmsg(db));
        _return(ctx, 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt) == SQLITE_DONE;
    if (!res) {
        printf("[ProxyRecomp_KV] Failed REMOVE %s: %s\n", key.c_str(), sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    _return(ctx, res);
}

}
