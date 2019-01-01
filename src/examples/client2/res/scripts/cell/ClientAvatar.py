"This module implements the Client Avatar entity."

import BigWorld

class ClientAvatar( BigWorld.Entity ):
	"An Avatar entity for the Client."

	def __init__( self ):
		BigWorld.Entity.__init__( self )

	def dummyFunctionNoParams( self, src ):
		print "Client.dummyFunctionNoParams:", self.id, src

# ClientAvatar.py
