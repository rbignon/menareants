#! /usr/bin/env python
#
# KIKOO LOL ASV

import types
import string, random, re
from ircbot import SingleServerIRCBot
from irclib import nm_to_n, nm_to_h, ip_numstr_to_quad, ip_quad_to_numstr
import asyncore, socket
from threading import Thread

irc_server = "irc.freenode.org"
irc_channel = "#menareants"
irc_nickname = "MenAreAnts2"
ms_server = "localhost"
ms_port = 5460

def SplitBuf(buf):

	"""
	slash = False
	parv = []
	i = 0
	while i < len(buf):
		if buf[i] == ' ': i = i + 1
		slash = False
		s = ""
		while i < len(buf) and (buf[i] != ' ' or slash):
			if buf[i] == '\\' and i+1 < len(buf) and (buf[i+1] == ' ' or buf[i+1] == '\\') and slash == False:
				slash = True
			else:
				s += buf[i]
				slash = False
			i = i + 1
		parv.append(s)
		i = i + 1

	return parv
	"""
	return [item.replace("\\ ", " ") for item in re.split(r"(?<!\\) ", buf)]

class MyThread(Thread):

	def __init__(self, bot):
		Thread.__init__(self)
		self.bot = bot

	def run(self):
		while self.bot.joined != True:
			pass
		self.ms = MetaServer((ms_server, ms_port), self.bot)
		self.bot.metaserver = self.ms
		asyncore.loop()

	def stop(self):
		self.ms.close()

class MetaServer (asyncore.dispatcher):

	def m_ping(self, parv):
		self.SendMsg("E")

	def m_hello(self, parv):
		self.SendMsg("B dd IRCBOT 3")

	def m_server(self, parv):
		"""LSP <ip:port> <nom> <+/-> <nbjoueurs>                                """
                """     <nbmax> <nbgames> <maxgames> <nbwaitgames> <proto>              """
                """     <version> <totusers> <totgames> <uptime>                        """
		if(int(parv[6]) >= int(parv[7]) or int(parv[4]) >= int(parv[5])):
			color = 4
		if(int(parv[6])):
			color = 2
		else:
			color = 3
		self.bot.SendMessage("\002%-25s\002 (\003%d\002\002%s/%s\003 players) (\003%d\002\002%s/%s\003 games) v%s"
		                     % (parv[2], color, parv[4], parv[5], color, parv[6], parv[7], parv[10]))

	def m_regnick(self, parv):
		""" SCORE <nick> <deaths> <killed> <creations> <scores> <best_revenu> <nbgames> <victories> <regtime> <lastvisit> """
		self.bot.top5_nb += 1
		self.bot.SendMessage("%d. \002%-15s\002 \0032\002\002%10s\003 (\0033+%02d\003|\0034\002\002%02d-\003) (\0033\002\002%7s\003 lost |\0033\002\002%7s\003 killed |\0033\002\002%7s\003 created |\0033\002\002%6s\003 $/t max)"
		                     % (self.bot.top5_nb, parv[1], parv[5], int(parv[8]), (int(parv[7])-int(parv[8])), parv[2], parv[3], parv[4], parv[6]))


	def m_newgame(self, parv):
		""" P  <serveur> <game> <players> <map> <type> """
		type_game = ""
		if parv[5] == "1": type_game = "mission"
		elif parv[5] == "2": type_game = "skirmish"
		elif parv[5] == "3": type_game = "multiplayer game"
		self.bot.SendMessage("\002%s\002 - New %s (\002%s\002) on map \0033%s\003. Players: \0033%s\003" % (parv[1], type_game, parv[2], parv[4], parv[3]))

	def m_endgame(self, parv):
		""" Q <server> <name> <map> <winers> <losers> """
		self.bot.SendMessage("\002%s\002 - End of \002%s\002 (map: \0033%s\003)." % (parv[1], parv[2], parv[3]))
		self.bot.SendMessage("Winers are: \0033%s\003" % parv[4])
		self.bot.SendMessage("Losers are: \0034%s\003" % parv[5])

	def m_regged(self, parv):
		""" i <nickname> """
		self.bot.SendMessage("Welcome to new registered player \002%s\002!" % parv[1])

	def m_creategame(self, parv):
		""" T <server> <name> <player> """
		self.bot.SendMessage("\002%s\002 - \0033%s\003 have created \002%s\002" % (parv[1], parv[3], parv[2]))

	def __init__(self, host, bot):
		asyncore.dispatcher.__init__(self)
		self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self.connect(host)
		self.bot = bot
		self.recvbuf = ""
		self.commands = {"D": self.m_ping,
		                 "A": self.m_hello,
				 "g": self.m_server,
				 "f": self.m_regnick,
				 "P": self.m_newgame,
				 "Q": self.m_endgame,
				 "i": self.m_regged,
				 "T": self.m_creategame}

	def handle_connect(self):
		pass

	def handle_close(self):
		self.close()

	def handle_read(self):
		data = self.recv(8192)

		for i in data:
			if i == '\n':
				self.recvbuf = self.recvbuf[:len(self.recvbuf)-1]
				s = SplitBuf(self.recvbuf)
				if s[0] in self.commands:
					self.commands[s[0]](s)
				self.recvbuf = ""
			else:
				self.recvbuf += i

	def writable(self):
		pass

	def handle_write(self):
		#sent = self.send(self.buffer)
		#print 'S - %s' % self.buffer[:sent]
		#self.buffer = self.buffer[sent:]
		pass

	def SendMsg(self, buf):
		self.send('%s\r\n' % buf)

class TestBot(SingleServerIRCBot):
	def __init__(self, channel, nickname, server, port=6667):
		SingleServerIRCBot.__init__(self, [(server, port)], nickname, nickname + "`")
		self.channel = channel
		self.joined = False
		self.top5_nb = 0

	def on_welcome(self, c, e):
		c.join(self.channel)
		self.joined = True

	def send_privmsgu(self, nick, message):
        	if type(nick)==types.UnicodeType:
			nick = nick.encode("ascii")
		self.connection.privmsg(nick, message.encode("UTF-8"))

	def SendMessage(self, message):
		self.send_privmsgu(self.channel, message)

	def on_pubmsg(self, c, e):
		if(e.arguments()[0] == "!bye"):
			self.SendMessage("bye !")
			self.die()
			return True
		if(e.arguments()[0] == "!help"):
			self.connection.notice(nm_to_n(e.source()), "Commands: !servers !top5".encode("UTF-8"))
		if(e.arguments()[0] == "!top5"):
			if(self.top5_nb != 0 and self.top5_nb != 5):
				return True
			self.top5_nb = 0
			self.metaserver.SendMsg("f")
		if(e.arguments()[0] == "!servers"):
			self.SendMessage("Servers:")
			self.metaserver.SendMsg("g")

def main():

	bot = TestBot(irc_channel, irc_nickname, irc_server)

	thread = MyThread(bot)
	thread.start()

	try:
		bot.start();
	except KeyboardInterrupt:
		print "Interrompu (CTRL+C)."

	thread.stop()

if __name__ == "__main__":
	main()
