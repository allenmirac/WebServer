# all:server
# .PHONY: all clean
# CXX ?= g++ #如果您希望仅在变量尚未设置时才将其设置为值,使用?=

# DEBUG ?= 1
# ifeq ($(DEBUG), 1)
#     CXXFLAGS += -g
# else
#     CXXFLAGS += -O2

# endif

# SERVER_OBJ: main.o\
# 	timer/lst_timer.o\
# 	http/httpconn.o\
# 	log/log.o\
# 	CGImysql/connpool.o\
# 	webserver.o\
# 	Epoll/epoll.o\
# 	InetAddress/inetaddress.o\
# 	util/util.o\
# 	threadpool/threadpool.h\
# 	config.o\

# server:${SERVER_OBJ}
# 	$(CXX)  -o server ${SERVER_OBJ} $^ $(CXXFLAGS) -lpthread -I/usr/include/cppconn -lmysqlcppconn
# # $^ = main.cc  ./timer/lst_timer.cc ./http/httpconn.cc ./log/log.cc ./CGImysql/connpool.cc  webserver.cc Epoll/epoll.cc InetAddress/inetaddress.cc util/util.cc threadpool/threadpool.h config.cc
# clean:
# 	rm -f ${SERVER_OBJ}

.PHONY: servers 
CXX ?= g++ #如果您希望仅在变量尚未设置时才将其设置为值,使用?=

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cc  ./timer/lst_timer.cc ./http/httpconn.cc ./log/log.cc ./CGImysql/connpool.cc  webserver.cc Epoll/epoll.cc InetAddress/inetaddress.cc util/util.cc threadpool/threadpool.h config.cc        
		$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -I/usr/include/cppconn -lmysqlcppconn
# $^ = main.cc  ./timer/lst_timer.cc ./http/httpconn.cc ./log/log.cc ./CGImysql/connpool.cc  webserver.cc Epoll/epoll.cc InetAddress/inetaddress.cc util/util.cc threadpool/threadpool.h config.cc
clean:
	rm  -r servers