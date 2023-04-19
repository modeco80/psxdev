add_executable(balls
    src/main.cpp
    # Nugget source files
    ${NUGGET}/common/crt0/crt0.s
)

#
#
#LDFLAGS += -Wl,--start-group
#LDFLAGS += -lapi
#LDFLAGS += -lc
#LDFLAGS += -letc
#LDFLAGS += -lgpu
#LDFLAGS += -lgte
#LDFLAGS += -lpad
#LDFLAGS += -Wl,--end-group
#
#

target_link_libraries(balls PRIVATE
    "-Wl,--start-group"
        api
        c
        etc
        gpu
        gte
        pad
    "-Wl,--end-group"
    ) 

psxdev_target(balls)