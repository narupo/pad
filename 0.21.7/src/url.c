#include "url.h"

struct cap_url {
    uint8_t host[CAP_URL_NHOST];
    int32_t port;
};

void
cap_urldel(struct cap_url *self) {
    if (self) {
        free(self);
    }
}

struct cap_url *
cap_urlnew(void) {
    struct cap_url *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }
    
    return self;
}

struct cap_url *
cap_urlparse(struct cap_url *self, const uint8_t *url) {
    if (!self || !url) {
        errno = EINVAL;
        return NULL;
    }

    char buf[CAP_URL_NBUF] = {0};
    uint32_t bi = 0, m = 0;

    self->host[0] = '\0';

    for (uint32_t i = 0; ; ++i) {
        uint8_t c = url[i]; // allow final nil

        switch (m) {
        case 0:
            switch (c) {
            case ':':
                buf[bi] = '\0';
                memmove(self->host, buf, bi);
                bi = 0;
                m = 1;
                break;
            case '\0':
                buf[bi] = '\0';
                memmove(self->host, buf, bi);
                break;
            default:
                buf[bi++] = c;
                break;
            }  
            break;
        case 1:
            switch (c) {
            case '\0':
                buf[bi] = '\0';
                self->port = atoi(buf);
                break;
            default:
                buf[bi++] = c;
                break;
            }
            break;
        default:
            assert(0 && "impossible");
            break;
        }

        if (c == '\0') {
            break;
        }
    }

    return self;
}

const uint8_t *
cap_urlhost(const struct cap_url *self) {
    if (!self) {
        errno = EINVAL;
        return NULL;
    }
    return self->host;
}

int32_t
cap_urlport(const struct cap_url *self) {
    if (!self) {
        errno = EINVAL;
        return -1;
    }
    return self->port;

}
