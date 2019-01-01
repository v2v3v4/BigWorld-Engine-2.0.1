"""
The ClientAvatar Client Entity script. 

Implements the client method fromCell() which is called from the ClientAvatar
cell entity.
"""
import BigWorld

# ------------------------------------------------------------------------------
# Section: class ClientAvatar
# ------------------------------------------------------------------------------
class ClientAvatar( BigWorld.Entity ):
	def __init__( self ):
		BigWorld.Entity.__init__( self )
		print "ClientAvatar.__init__:"
		print self.__dict__

	def onFinish( self ):
		self.base.logOff()

	def fromCell( self, msg ):
		print "ClientAvatar.fromCell:", self.id, ":", msg

	def onTick( self, t ):
		return
#		print t
#		print self.prop5.intValue

# ClientAvatar.py
