#include "server.h"

static int validate_sign_in(sqlite3 *db, const char *name, const char *passw) {
    t_user *user = mx_get_user_by_login(db, (char *)name);

    if (!user || mx_strcmp((char *)passw, user->user_pass))
        return -1;
    else
        return user->user_id;
}

static char *add_to_db(sqlite3 *db, char *name, char *passw) {
    int code = mx_add_user(db, (char *)name, (char *)passw);

    if (code > 0)
       return "{\"code\": 200}";
    else if (code == -1)
       return "{\"code\": 500}";
    else //  code == -2, username already exists
       return "{\"code\": 409}";
}

static int extract_name_passw(json_object *json, const char **name,
                              const char **passw) {
    json_object *j_name = NULL;
    json_object *j_passw = NULL;

    json_object_object_get_ex(json, "name", &j_name);
    json_object_object_get_ex(json, "passw", &j_passw);

    if (j_name && j_passw && json_object_get_type(j_name) == json_type_string
        && json_object_get_type(j_passw) == json_type_string) {
        *name = json_object_get_string(j_name);
        *passw = json_object_get_string(j_passw);
    }

    if (*name == NULL || *passw == NULL)
        return 1;
    else
        return 0;
}

char *mx_register_user(void *jobj, sqlite3 *db, int fd) {
    const char *name = NULL;
    const char *pass = NULL;
    int code = 0;

    fd = 0;

    if ((code = extract_name_passw((json_object *)jobj, &name, &pass)) != 0)
        return mx_bad_request(NULL, NULL, 0);

    return add_to_db(db, (char *)name, (char *)pass);
}

char *mx_sign_in(void *jobj, sqlite3 *db, int fd) {
    const char *name = NULL;
    const char *pass = NULL;
    int code = 0;
    int uid = -1;
    char *token = NULL;

    fd = 0;
    if ((code = extract_name_passw((json_object *)jobj, &name, &pass)) != 0)
        return mx_bad_request(NULL, NULL, 0);

    if ((uid = validate_sign_in(db, name, pass)) == -1)
        return "{\"code\": 401}";
    // generate token
    if (mx_add_sock_user(db, uid, fd, token) == -1)
        return "{\"code\": 500}";
    return "{\"code\": 200}";
}
