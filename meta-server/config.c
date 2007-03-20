/* meta-server/config.h - Configuration of Meta Server
 *
 * Copyright (C) 2007 Romain Bignon  <Progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: CheckBox.h 907 2007-02-13 19:46:20Z progs $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "config.h"

#define CF_REASONLEN 300

struct Config config;
struct ConfigTab;
struct ConfigItem;

int cf_add_server (struct ConfigTab* section);
int cf_check_port (struct ConfigItem*, char*);
int cf_check_ip   (struct ConfigItem*, char*);

char *str_dup(char **to, const char *from)
{
        if(from && *from)
        {
                *to = realloc(*to, strlen(from) + 1);
                if(!*to) printf("strdup: no memory for 'to'(%p)=%s 'from'(%p)=%s\n",
                        (void *) *to, *to, (const void *) from, from);
                else strcpy(*to, from);
        }
        return *to;
}

char *Strncpy(char *to, const char *from, size_t n) /* copie from dans to. ne copie que n char */
{                                                       /* MAIS AJOUTE LE CHAR \0 � la fin, from DOIT donc faire n+1 chars. */
        const char *end = to + n;
        char *save = to;
        while(to < end && (*to++ = *from++));
        *to = 0;
        return save;
}


#define MAXITEMS 3
struct ConfigTab
{
	const char* name;
	const char* description;
	unsigned int flags;
	#define CONF_READ   0x01
	#define CONF_CLONE  0x02

	int (*add_tab) (struct ConfigTab*);

	struct ConfigItem
	{
		const char* name;
		const char* description;
		unsigned int flags;
		/*      CONF_READ   0x01 */
		#define CONF_TINT   0x02
		#define CONF_TFLAG  0x04
		#define CONF_TPTR   0x08
		#define CONF_TARRAY 0x10

		void* ptr;
		size_t psize;
		int (*check_value) (struct ConfigItem*, char*);

	} items[MAXITEMS];
} config_tab[] =
{
	{ "meta-server", "meta server parameters",                 0,                   0,
		{
			{ "port",      "port name of Meta Server", CONF_TINT,           &config.port,     0,   &cf_check_port },
			{ "pingfreq",  "frequence of pings",       CONF_TINT,           &config.pingfreq, 0,   0 }
		}
	},
	{ "server",      "",                                       CONF_CLONE,          &cf_add_server,
		{
			{ "name",      "server name",              CONF_TARRAY,         0,                0,   0 },
			{ "ip",        "server ip",                CONF_TARRAY,         0,                0,   &cf_check_ip },
			{ "passwd",    "server password",          CONF_TARRAY,         0,                0,   0 }
		}
	}
};

static struct ConfigItem* get_item(struct ConfigTab* section, const char* name)
{
	unsigned int i = 0;
	for(; i < MAXITEMS; ++i)
		if(!strcasecmp(section->items[i].name, name))
			return &section->items[i];

	return 0;
}

int cf_add_server (struct ConfigTab* section)
{
	struct ServerConfig* serv = malloc(sizeof *serv);
	serv->name[0] = 0;
	section->items[0].ptr = serv->name;
	section->items[0].psize = sizeof serv->name - 1;
	serv->ip[0] = 0;
	section->items[1].ptr = serv->ip;
	section->items[1].psize = sizeof serv->ip - 1;
	serv->password[0] = 0;
	section->items[2].ptr = serv->password;
	section->items[2].psize = sizeof serv->password - 1;

	serv->next = config.servers;
	config.servers = serv;
	return 0;
}

int cf_check_port (struct ConfigItem* i, char* buf)
{
	int port = *(int *) i->ptr;
	if(port > 0 && port <= 65535) return 1;
	snprintf(buf, CF_REASONLEN, "item %s: %d isn't a valid port number (1-65535).\n", i->name, port);
	return 0;
}

int is_num(const char *num)
{
   while(*num) if(!isdigit(*num++)) return 0;
   return 1;
}

int cf_check_ip(struct ConfigItem* item, char* buf)
{
	char *ip = (char*) item->ptr;
	char *ptr = NULL;
	int i = 0, d = 0;

	for(; i < 4; ++i) /* 4 dots expected (IPv4) */
	{       /* Note about strtol: stores in endptr either NULL or '\0' if conversion is complete */
		if(!isdigit((unsigned char) *ip) /* most current case (not ip, letter host) */
		    || (d = strtol(ip, &ptr, 10)) < 0 || d > 255 /* ok, valid number? */
		    || (ptr && *ptr != 0 && (*ptr != '.' || 3 == i) && ptr != ip))
		{
			snprintf(buf, CF_REASONLEN, "item %s: %s isn't a valid IP.\n", item->name, ip);
			return 0;
		}
		if(ptr) ip = ptr + 1, ptr = NULL; /* jump the dot */
	}
	return 1;
}

static struct ConfigTab *get_tab(const char *item)
{
	unsigned int i = 0;
	for(; i < ASIZE(config_tab); ++i)
		if(!strcasecmp(config_tab[i].name, item))
			return &config_tab[i];
	return NULL;
}

static int readconf (FILE* fp)
{
	int line = 0;
	char buf[512], vreason[CF_REASONLEN + 1];
	struct ConfigTab* section = 0;

	while(fgets(buf, sizeof buf, fp))
	{
		char* ptr = buf;
		++line;

		while(*ptr == '\t' || *ptr == ' ') ++ptr;

		if(*ptr == '#' || *ptr == '\r' || *ptr == '\n' || !*ptr) continue;

		if(!section)
		{
			ptr = strtok(ptr, " ");
			if(!(section = get_tab(ptr)))
			{
				printf("conf(%d): Unknown section '%s'\n", line, ptr);
				continue;
			}
			if((section->flags & CONF_READ) && !(section->flags & CONF_CLONE))
			{
				printf("conf(%d): Section '%s' is already read\n", line, ptr);
				section = 0;
				continue;
			}
			section->flags |= CONF_READ;
			if(section->add_tab)
				section->add_tab (section);

		}
		else if(strchr(ptr, '='))
		{
			const char* label = strtok(ptr, " ");
			struct ConfigItem* item = get_item(section, label);
			if(!item)
			{
				printf("conf(%d): Unknown item '%s'\n", line, label);
				continue;
			}
			if(!*ptr)
			{
				printf("conf(%d): There isn't any value\n", line);
				continue;
			}
			const char* value = strtok(NULL, "=\r\n");
			while(value && *value && (*value == ' ' || *value == '=' || *value == '\t')) ++value;
			if(!value || !*value)
			{
				printf("conf(%d): There isn't any value\n", line);
				continue;
			}

			if(item->flags & CONF_TINT && !is_num(value))
			{
				printf("conf(%d): Item '%s' might be a number (%s).\n", line, item->name, value);
				return -1;
			}

			if(item->flags & CONF_TARRAY)
				Strncpy((char *) item->ptr, value, item->psize);
			else if(item->flags & CONF_TPTR) str_dup((char **) item->ptr, value);
			else if(item->flags & CONF_TFLAG)
			{
				if(atoi(ptr)) *(int *)item->ptr |= item->psize;
				else *(int *)item->ptr &= ~item->psize;
			}
			else if(item->flags & CONF_TINT)
				*(int *)item->ptr = atoi(value);
			else
			{
				printf("conf(%d): item '%s' hasn't any type !?\n", line, item->name);
				return -1;
			}


			if(item->check_value && !item->check_value(item, vreason))
			{
				printf("conf(%d): %s", line, vreason);
				return -1;
			}
		}
		else if(*ptr && *ptr == '}')
		{
			if(!section)
			{
				printf("conf(%d): a '}' out of a section !?\n", line);
				continue;
			}

			section = 0;
		}
	}

	if(section)
	{
		printf("in «%s»: '}' not found to close section !\n", section->name);
		return -1;
	}

	return 1;
}

int load_config(const char* path)
{
	FILE *fp = fopen(path, "r");
	int error = 1;

	config.pingfreq = DEFPINGFREQ;
	config.port = DEFPORT;
	config.servers = 0;

	if(fp)
	{
		error = readconf(fp);
		fclose(fp);
	}
	else printf("conf: Unable to open configuration file (%s)\n"
	            "conf: We will start without any configuration, but please copy metaserver.conf from sources to %s\n", strerror(errno), path);

	return error;
}

int rehash_config(const char* path)
{
	struct ServerConfig* s = config.servers;
	unsigned int i = 0;

	while(s)
	{
		struct ServerConfig* ss = s->next;
		free(s);
		s = ss;
	}

	for(; i < ASIZE(config_tab); ++i)
		config_tab[i].flags &= ~CONF_READ;

	return load_config(path);
}
