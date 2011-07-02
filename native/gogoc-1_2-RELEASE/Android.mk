LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
		gogoc-pal/platform/common/src/pal_version.c \
		gogoc-messaging/src/gogocuistrings.c \
		gogoc-tsp/src/lib/base64.c \
		gogoc-tsp/src/lib/cli.c \
		gogoc-tsp/src/lib/config.c \
		gogoc-tsp/src/lib/lib.c \
		gogoc-tsp/src/lib/log.c \
		gogoc-tsp/src/lib/md5c.c \
		gogoc-tsp/src/lib/buffer.c \
		gogoc-tsp/src/lib/version.c \
		gogoc-tsp/src/lib/dns.c \
		gogoc-tsp/src/lib/deque.c \
		gogoc-tsp/src/net/net.c \
		gogoc-tsp/src/net/net_rudp.c \
		gogoc-tsp/src/net/net_rudp6.c \
		gogoc-tsp/src/net/net_tcp.c \
		gogoc-tsp/src/net/net_udp.c \
		gogoc-tsp/src/net/net_ka.c \
		gogoc-tsp/src/net/net_cksm.c \
		gogoc-tsp/src/net/net_tcp6.c \
		gogoc-tsp/src/net/net_echo_request.c \
		gogoc-tsp/src/net/icmp_echo_engine.c \
		gogoc-tsp/src/tsp/tsp_auth.c \
		gogoc-tsp/src/tsp/tsp_cap.c \
		gogoc-tsp/src/tsp/tsp_client.c \
		gogoc-tsp/src/tsp/tsp_net.c \
		gogoc-tsp/src/tsp/tsp_setup.c \
		gogoc-tsp/src/tsp/tsp_lease.c \
		gogoc-tsp/src/tsp/tsp_redirect.c \
		gogoc-tsp/src/tsp/tsp_tun_mgt.c \
		gogoc-tsp/src/xml/xmlparse.c \
		gogoc-tsp/src/xml/xml_req.c \
		gogoc-tsp/src/xml/xml_tun.c \
		gogoc-tsp/platform/unix-common/unix-main.c \
		gogoc-tsp/platform/linux/tsp_local.c \
		gogoc-tsp/platform/linux/tsp_tun.c

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/gogoc-pal/defs \
		$(LOCAL_PATH)/gogoc-pal/out_inc \
		$(LOCAL_PATH)/gogoc-pal/platform/unix-common/inc \
		$(LOCAL_PATH)/gogoc-pal/platform/common/inc \
		$(LOCAL_PATH)/gogoc-tsp/platform/linux \
		$(LOCAL_PATH)/gogoc-tsp/include \
		$(LOCAL_PATH)/gogoc-config \
		$(LOCAL_PATH)/gogoc-messaging


LOCAL_CFLAGS = \
		-DPLATFORM=\"linux\" \
		-D_REENTRANT \
		-DNDEBUG \
		-DINET_ADDRSTRLEN=16

LOCAL_CFLAGS += -Wall -Wextra

ifeq ($(HAVE_OPENSSL),true)
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_SHARED_LIBRARIES += libcrypto
LOCAL_SRC_FILES += \
		gogoc-tsp/src/lib/bufaux.c \
		gogoc-tsp/src/tsp/tsp_auth_passdss.c
else
LOCAL_CFLAGS +=	-DNO_OPENSSL
endif

LOCAL_MODULE := gogoc
LOCAL_SYSTEM_SHARED_LIBRARIES := libc
# LOCAL_STATIC_LIBRARIES := libc
# LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_BIN)
include $(BUILD_EXECUTABLE)
