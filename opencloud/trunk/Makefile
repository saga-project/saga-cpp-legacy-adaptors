
-include config/make.cfg


SAGA_SUBDIRS    = config external


ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
  SAGA_SUBDIRS += opencloud_context
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_FILE)" "xyes"
  SAGA_SUBDIRS += opencloud_file
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += opencloud_job
endif

ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS += opencloud_file opencloud_job
endif


include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk


external:          config
opencloud_context: external
opencloud_file:    opencloud_context
opencloud_job:     opencloud_context

