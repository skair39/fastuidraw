# Begin standard header
sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)
# End standard header

LIBRARY_SOURCES += $(call filelist, \
	painter_clip_equations.cpp painter_item_matrix.cpp \
	painter_header.cpp painter_packer.cpp \
	painter_backend.cpp painter_draw.cpp)

# Begin standard footer
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
# End standard footer
