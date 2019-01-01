import BigWorld
import whrandom

# ------------------------------------------------------------------------------
# Section: class Guard
# ------------------------------------------------------------------------------

class Guard( BigWorld.Base ):

	def __init__( self ):
		BigWorld.Base.__init__( self )
		print "Base: Guard.__init__"
		self.cellData[ "position" ] = (whrandom.randrange( -1, 1 ), 0,
				whrandom.randrange( -1, 1 ))
								
		self.createInDefaultSpace()

# Guard.py
