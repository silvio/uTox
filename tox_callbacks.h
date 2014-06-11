static void* copy_message(uint8_t *str, uint16_t length, uint16_t flags)
{
    length = utf8_validate(str, length);

    MESSAGE *msg = malloc(length + 6);
    msg->flags = flags;
    msg->length = length;
    memcpy(msg->msg, str, length);

    return msg;
}

static void* copy_groupmessage(Tox *tox, uint8_t *str, uint16_t length, uint16_t flags, int gid, int pid)
{
    uint8_t name[TOX_MAX_NAME_LENGTH];
    int namelen = tox_group_peername(tox, gid, pid, name);
    if(namelen == 0 || namelen == -1) {
        memcpy(name, "<unknown>", 9);
        namelen = 9;
    }

    length = utf8_validate(str, length);
    namelen = utf8_validate(name, namelen);


    MESSAGE *msg = malloc(6 + length + namelen);
    msg->flags = flags;
    msg->length = length;
    memcpy(msg->msg, str, length);

    msg->msg[length] = (char_t)namelen;
    memcpy(&msg->msg[length] + 1, name, namelen);

    return msg;
}

static void callback_friend_request(Tox *tox, uint8_t *id, uint8_t *msg, uint16_t length, void *userdata)
{
    length = utf8_validate(msg, length);

    FRIENDREQ *req = malloc(sizeof(FRIENDREQ) - 1 + length);

    req->length = length;
    memcpy(req->id, id, sizeof(req->id));
    memcpy(req->msg, msg, length);

    postmessage(FRIEND_REQUEST, 0, 0, req);

    /*int r = tox_add_friend_norequest(tox, id);
    void *data = malloc(TOX_FRIEND_ADDRESS_SIZE);
    memcpy(data, id, TOX_FRIEND_ADDRESS_SIZE);

    postmessage(FRIEND_ACCEPT, (r < 0), (r < 0) ? 0 : r, data);*/
}

static void callback_friend_message(Tox *tox, int fid, uint8_t *message, uint16_t length, void *userdata)
{
    postmessage(FRIEND_MESSAGE, fid, 0, copy_message(message, length, 0));

    debug("Friend Message (%u): %.*s\n", fid, length, message);
}

static void callback_friend_action(Tox *tox, int fid, uint8_t *action, uint16_t length, void *userdata)
{
    postmessage(FRIEND_MESSAGE, fid, 0, copy_message(action, length, 2));

    debug("Friend Action (%u): %.*s\n", fid, length, action);
}

static void callback_name_change(Tox *tox, int fid, uint8_t *newname, uint16_t length, void *userdata)
{
    length = utf8_validate(newname, length);

    void *data = malloc(length);
    memcpy(data, newname, length);

    postmessage(FRIEND_NAME, fid, length, data);

    debug("Friend Name (%u): %.*s\n", fid, length, newname);
}

static void callback_status_message(Tox *tox, int fid, uint8_t *newstatus, uint16_t length, void *userdata)
{
    length = utf8_validate(newstatus, length);

    void *data = malloc(length);
    memcpy(data, newstatus, length);

    postmessage(FRIEND_STATUS_MESSAGE, fid, length, data);

    debug("Friend Status Message (%u): %.*s\n", fid, length, newstatus);
}

static void callback_user_status(Tox *tox, int fid, uint8_t status, void *userdata)
{
    postmessage(FRIEND_STATUS, fid, status, NULL);

    debug("Friend Userstatus (%u): %u\n", fid, status);
}

static void callback_typing_change(Tox *tox, int fid, uint8_t is_typing, void *userdata)
{
    postmessage(FRIEND_TYPING, fid, is_typing, NULL);

    debug("Friend Typing (%u): %u\n", fid, is_typing);
}

static void callback_read_receipt(Tox *tox, int fid, uint32_t receipt, void *userdata)
{
    //postmessage(FRIEND_RECEIPT, fid, receipt);

    debug("Friend Receipt (%u): %u\n", fid, receipt);
}

static void callback_connection_status(Tox *tox, int fid, uint8_t status, void *userdata)
{
    postmessage(FRIEND_ONLINE, fid, status, NULL);

    debug("Friend Online/Offline (%u): %u\n", fid, status);
}

static void callback_group_invite(Tox *tox, int fid, uint8_t *group_public_key, void *userdata)
{
    int gid = tox_join_groupchat(tox, fid, group_public_key);
    if(gid != -1) {
        postmessage(GROUP_ADD, gid, 0, NULL);
    }

    debug("Group Invite (%i,f:%i)\n", gid, fid);
}

static void callback_group_message(Tox *tox, int gid, int pid, uint8_t *message, uint16_t length, void *userdata)
{
    postmessage(GROUP_MESSAGE, gid, 0, copy_groupmessage(tox, message, length, 0, gid, pid));

    debug("Group Message (%u, %u): %.*s\n", gid, pid, length, message);
}

static void callback_group_action(Tox *tox, int gid, int pid, uint8_t *action, uint16_t length, void *userdata)
{
    postmessage(GROUP_MESSAGE, gid, 0, copy_groupmessage(tox, action, length, 2, gid, pid));

    debug("Group Action (%u, %u): %.*s\n", gid, pid, length, action);
}

static void callback_group_namelist_change(Tox *tox, int gid, int pid, uint8_t change, void *userdata)
{
    switch(change) {
    case TOX_CHAT_CHANGE_PEER_ADD: {
        postmessage(GROUP_PEER_ADD, gid, pid, NULL);
        break;
    }

    case TOX_CHAT_CHANGE_PEER_DEL: {
        postmessage(GROUP_PEER_DEL, gid, pid, NULL);
        break;
    }

    case TOX_CHAT_CHANGE_PEER_NAME: {
        uint8_t name[TOX_MAX_NAME_LENGTH];
        int len = tox_group_peername(tox, gid, pid, name);

        len = utf8_validate(name, len);

        void *data = malloc(len + 1);
        *(uint8_t*)data = len;
        memcpy(data + 1, name, len);

        postmessage(GROUP_PEER_NAME, gid, pid, data);
        break;
    }
    }
    debug("Group Namelist Change (%u, %u): %u\n", gid, pid, change);
}