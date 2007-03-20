INCLUDES = -I$(top_srcdir)/lib

bin_PROGRAMS = menareants-meta-server
menareants_meta_server_SOURCES = \
	main.c \
	config.c \
	database.c \
	sockets.c \
	servers.c \
	clients.c

man6_MANS = menareants-meta-server.6
EXTRA_DIST = $(man6_MANS)

clean-generic:
	-rm -f .#* *~
