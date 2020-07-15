#include "server.h"

char *mx_msg_json_builder(t_msg *msg) {
    json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "code", json_object_new_int(200));
    mx_j_o_o_a(jobj, "msg", mx_json_builder_msg(msg));

    return (char *)json_object_to_json_string(jobj);
}

char *mx_json_string_notify_gr(int gid, char *gr_name, int type) {
    json_object *jobj = json_object_new_object();

    mx_j_o_o_a(jobj, "type", json_object_new_int(type));
    mx_j_o_o_a(jobj, "gid", json_object_new_int(gid));
    mx_j_o_o_a(jobj, "name", json_object_new_string(gr_name));

    return (char *)json_object_to_json_string(jobj);
}
