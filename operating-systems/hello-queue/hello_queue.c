#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minix/ds.h>
#include <minix/ioctl.h>
#include <sys/ioc_hello_queue.h>
#include "hello_queue.h"

#define START_STR "queue_start"
#define END_STR "queue_end"
#define SIZE_STR "queue_alloc_size"
#define QUEUE_STR "hello_queue"


/* Auxiliary functions. */
static void check_alloc();
static void init_queue();
static void resize_queue(size_t size);
static void move_to_buf_start();
static void put_string(char str[MSG_SIZE]);
static void replace_all_occurrences(char replace[2]);
static void delete_every_third();

/* Chardriver functions. */
static int hello_queue_open(devminor_t minor, int access, endpoint_t user_endpt);
static int hello_queue_close(devminor_t minor);
static ssize_t hello_queue_read(devminor_t minor, u64_t position, endpoint_t endpt,
                            cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t hello_queue_write(devminor_t minor, u64_t position, endpoint_t endpt,
                            cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int hello_queue_ioctl(devminor_t UNUSED(minor), unsigned long request,
                             endpoint_t endpt, cp_grant_id_t grant, int UNUSED(flags),
                             endpoint_t user_endpt, cdev_id_t UNUSED(id));

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);


/* Entry points to the hello queue driver. */
static struct chardriver hello_queue_tab =
{
    .cdr_open	= hello_queue_open,
    .cdr_close	= hello_queue_close,
    .cdr_read	= hello_queue_read,
    .cdr_write  = hello_queue_write,
    .cdr_ioctl  = hello_queue_ioctl,
};

static ssize_t start;
static ssize_t end;
static ssize_t alloc_size = 0;
static char *queue = NULL;


static void check_alloc() {
    if (!queue) {
        printf("Memory alloc failed\n");
        exit(1);
    }
}

static void init_queue() {
    if (alloc_size) {
        free(queue);
    }
    start = 0;
    end = DEVICE_SIZE;
    alloc_size = DEVICE_SIZE;
    queue = (char *) calloc(alloc_size, sizeof(char));
    check_alloc();

    for (int i = 0; i < DEVICE_SIZE; ++i) {
        queue[i] = 'x' + (i % 3);
    }
}

static void resize_queue(size_t size) {
    while (end + size > alloc_size) {
        alloc_size *= 2;
        queue = realloc(queue, alloc_size);
    }
}

static void move_to_buf_start() {
    ssize_t queue_size = (end - start);
    memcpy(queue, queue + start, queue_size);
    start = 0;
    end = queue_size;
}

static void put_string(char str[MSG_SIZE]) {
    ssize_t queue_size = (end - start);

    if (MSG_SIZE <= alloc_size) {
        if (MSG_SIZE <= queue_size) {
            memcpy(queue + (queue_size - MSG_SIZE), str, MSG_SIZE);
        }
        else {
            memcpy(queue, str, MSG_SIZE);
            end = MSG_SIZE;
        }
    }
    else {
        resize_queue(MSG_SIZE);
        memcpy(queue, str, MSG_SIZE);
        end = MSG_SIZE;
    }
}

static void replace_all_occurrences(char replace[2]) {
    ssize_t queue_size = (end - start);

    for (int i = 0; i < queue_size; ++i) {
        if (queue[i] == replace[0]) {
            queue[i] = replace[1];
        }
    }
}

static void delete_every_third() {
    ssize_t tail = 0, queue_size = (end - start);

    for (int i = 0; i < queue_size; ++i) {
        if (i % 3 != 2) {
            queue[tail] = queue[i];
            ++tail;
        }
    }
    end = tail;
}

static int hello_queue_open(devminor_t UNUSED(minor), int UNUSED(access),
        endpoint_t UNUSED(user_endpt)) {
    return OK;
}

static int hello_queue_close(devminor_t UNUSED(minor)) {
    return OK;
}

static ssize_t hello_queue_read(devminor_t minor, u64_t position, endpoint_t endpt,
                            cp_grant_id_t grant, size_t size, int flags, cdev_id_t id) {
    int ret;
    ssize_t queue_size = (end - start);

    if (queue_size == 0 || size == 0) {
        return 0;
    }

    size = (size_t) (queue_size >= size ? size : queue_size);
    start = (size_t) (start + size);
    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) queue, size)) != OK) {
        size = ret;
    }

    move_to_buf_start();
    if (4 * queue_size <= alloc_size) {
        alloc_size /= 2;
        queue = realloc(queue, alloc_size);
    }

    return size;
}

static ssize_t hello_queue_write(devminor_t minor, u64_t position, endpoint_t endpt,
                            cp_grant_id_t grant, size_t size, int flags, cdev_id_t id) {
    int ret, offset = 0, bytes_left = size, copy_size;
    char buf[128];

    resize_queue(size);

    while (bytes_left > 0) {
        copy_size = bytes_left >= sizeof(buf) ? sizeof(buf) : bytes_left;
        bytes_left -= copy_size;
        ret = sys_safecopyfrom(endpt, grant, offset, (vir_bytes) buf, copy_size);
        if (ret != OK) {
            copy_size = ret;
        }
        memcpy(queue + end + offset, buf, copy_size);
        offset += copy_size;
    }
    end += size;

    return size;
}

static int hello_queue_ioctl(devminor_t UNUSED(minor), unsigned long request,
                             endpoint_t endpt, cp_grant_id_t grant, int UNUSED(flags),
                             endpoint_t user_endpt, cdev_id_t UNUSED(id)) {
    int ret = OK;
    char buf2[2], buf_msg[MSG_SIZE];

    switch (request) {
        case HQIOCRES: // Restore initial state.
            init_queue();
            break;
        case HQIOCSET:
            ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf_msg, MSG_SIZE);
            if (ret == OK) {
                put_string(buf_msg);
            }
            break;
        case HQIOCXCH: // Replace all occurrences of first with second.
            ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buf2, 2);
            if (ret == OK) {
                replace_all_occurrences(buf2);
            }
            break;
        case HQIOCDEL: // Delete every third element.
            delete_every_third();
            break;
        default:
            ret = ENOTTY;
    }

    return ret;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32(START_STR, start, DSF_OVERWRITE);
    ds_publish_u32(END_STR, end, DSF_OVERWRITE);
    ds_publish_u32(SIZE_STR, alloc_size, DSF_OVERWRITE);
    ds_publish_mem(QUEUE_STR, queue, alloc_size, DSF_OVERWRITE);
    free(queue);

    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    u32_t value1;
    size_t value2;

    ds_retrieve_u32(START_STR, &value1);
    ds_delete_u32(START_STR);

    start = (ssize_t) value1;
    ds_retrieve_u32(END_STR, &value1);
    ds_delete_u32(END_STR);

    end = (ssize_t) value1;
    ds_retrieve_u32(SIZE_STR, &alloc_size);
    ds_delete_u32(SIZE_STR);

    queue = calloc(alloc_size, sizeof(char));
    check_alloc();

    ds_retrieve_mem(QUEUE_STR, queue, &value2);
    ds_delete_mem(QUEUE_STR);

    return OK;
}

static void sef_local_startup() {
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info)) {
    /* Initialize the hello queue driver. */
    int do_announce_driver = TRUE;

    switch(type) {
        case SEF_INIT_FRESH:
            init_queue();
            break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
            break;

        case SEF_INIT_RESTART:
            lu_state_restore();
            break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

int main(void) {
    if (DEVICE_SIZE <= 0) {
        printf("Incorrect device size\n");
        exit(1);
    }
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /*
     * Run the main loop.
     */
    chardriver_task(&hello_queue_tab);
    free(queue);
    return OK;
}