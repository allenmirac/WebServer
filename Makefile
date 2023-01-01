.PHONY: servers 
CXX ?= g++ #如果您希望仅在变量尚未设置时才将其设置为值,使用?=

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

servers: main.cc  ./timer/lst_timer.cc ./http/httpconn.cc ./log/log.cc ./CGImysql/connpool.cc  webserver.cc Epoll/epoll.cc InetAddress/inetaddress.cc util/util.cc threadpool/threadpool.cc config.cc        
		$(CXX) -o servers  $^ $(CXXFLAGS) -lpthread -I/usr/include/cppconn -lmysqlcppconn
# $^ = main.cc  ./timer/lst_timer.cc ./http/httpconn.cc ./log/log.cc ./CGImysql/connpool.cc  webserver.cc Epoll/epoll.cc InetAddress/inetaddress.cc util/util.cc threadpool/threadpool.cc config.cc
clean:
	rm  -r servers