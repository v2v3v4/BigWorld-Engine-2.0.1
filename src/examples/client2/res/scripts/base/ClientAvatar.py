import BigWorld

# ------------------------------------------------------------------------------
# Section: class ClientAvatar
# ------------------------------------------------------------------------------

class ClientAvatar( BigWorld.Proxy ):

	def __init__( self ):
		BigWorld.Proxy.__init__( self )
		self.createInDefaultSpace()

	def onClientDeath( self ):
		print "ClientAvatar.onClientDeath:", self.id
		if hasattr( self, "cell" ):
			self.destroyCellEntity()
		else:
			self.destroy()
	
	def logOff( self ):
		print "ClientAvatar.logOff:", self.id
		if hasattr( self, "cell" ):
			self.destroyCellEntity()
		else:
			self.destroy()

	def onLoseCell( self ):
		self.destroy()

# ClientAvatar.py
