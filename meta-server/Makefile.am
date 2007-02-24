INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = menareants-meta-server
menareants_meta_server_SOURCES = \
	main.c \
	database.c \
	sockets.c \
	servers.c \
	clients.c

clean-generic:
	-rm -f .#* *~
