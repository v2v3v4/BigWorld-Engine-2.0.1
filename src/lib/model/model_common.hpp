/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MODEL_COMMON_HPP
#define MODEL_COMMON_HPP

namespace Moo
{

// Creates the right model class based on the file
// __kyl__ (20/10/2006) This method doesn't really save any code - in fact it
// might even increase the amount of code due to the need to implement FACTORY.
// But it creates a link between the client and server code so it makes people
// think twice before modifying this code.
template <class FACTORY>
typename FACTORY::ModelBase* createModelFromFile( DataSectionPtr& pFile,
		FACTORY& factory )
{
	if (pFile->openSection( "nodefullVisual" ))
	{
		return factory.newNodefullModel();
	}
	else if (pFile->openSection( "nodelessVisual" ))
	{
		return factory.newNodelessModel();
	}
	//else if (pFile->openSection( "billboardVisual" ))
	//{
	//	return factory.newBillboardModel();
	//}
	else
	{
		return NULL;
	}
}

} // namespace Moo

#endif // MODEL_COMMON_HPP
