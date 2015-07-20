#include <stdio.h>
#include <pulse/pulseaudio.h>
#include <pulse/error.h>

static void card_cb(pa_context *c, const pa_card_info *card, int eol, void *userdata) {
    if (eol > 0) {
        return;
    }

    pa_card_port_info *port = NULL;

    for (pa_card_port_info **po = card->ports; *po; po++) {
        if ((*po)->available != PA_PORT_AVAILABLE_NO) {
            if (!port || port->priority < (*po)->priority) {
                port = *po;
            }
        }
    }

    if (!port) {
        return;
    }

    pa_card_profile_info2 *profile = NULL;

    for (pa_card_profile_info2 **pr = port->profiles2; *pr; pr++) {
        if ((*pr)->available) {
            if (!profile || profile->priority < (*pr)->priority) {
                profile = *pr;
            }
        }
    }

    if (profile) {
        printf("Card %s: switch profile to %s\n", card->name, profile->name);
        pa_operation *op = pa_context_set_card_profile_by_name(c, card->name, profile->name, NULL, NULL);
        pa_operation_unref(op);
    }
}

static void event_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata) {
    pa_operation *op = pa_context_get_card_info_by_index(c, idx, card_cb, NULL);
    if (!op) {

    }
    pa_operation_unref(op);
}

static void state_cb(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            pa_context_set_subscribe_callback(c, event_cb, NULL);

            pa_operation *op = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_CARD, NULL, NULL);
            pa_operation_unref(op);
            break;

        case PA_CONTEXT_TERMINATED:
            exit(0);
            break;

        case PA_CONTEXT_FAILED:
        default:
            printf("Connection failure: %s", pa_strerror(pa_context_errno(c)));
            exit(2);
    }
}

int main(void) {

    pa_mainloop *loop = pa_mainloop_new();

    pa_mainloop_api *loop_api = pa_mainloop_get_api(loop);

    pa_context *c = pa_context_new(loop_api, "pa-switch-on-port-available");

    pa_context_set_state_callback(c, state_cb, NULL);

    pa_context_connect(c, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);

    int res;
    pa_mainloop_run(loop, &res);

    pa_context_disconnect(c);

    return res;
}

