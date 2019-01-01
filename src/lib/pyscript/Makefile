LIB =	pyscript

SRCS =					\
	frozen_modules		\
	personality			\
	pickler				\
	py_data_section		\
	py_output_writer	\
	py_traceback		\
	py_to_stl			\
	pyobject_plus		\
	py_import_paths		\
	pywatcher			\
	res_mgr_import		\
	res_mgr_script		\
	resource_table		\
	script				\
	script_math			\
	stl_to_py			\

ifndef MF_ROOT
export MF_ROOT := $(subst /bigworld/src/lib/$(LIB),,$(CURDIR))
endif

USE_PYTHON = 1

include $(MF_ROOT)/bigworld/src/build/common.mak
