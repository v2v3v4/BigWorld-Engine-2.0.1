/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_SPLITTER_HPP
#define VISUAL_SPLITTER_HPP

namespace Moo
{
	class Node;
	typedef SmartPointer<Node> NodePtr;

/**
 *	This class is a helper class that will split rendersets so that each one
 *	is affected by less than a set number of bones.
 */
class VisualSplitter
{
public:
	VisualSplitter(uint32 nodeLimit = 17);
	~VisualSplitter();

	void open( const std::string& resourceName );
	bool split();
	void save( const std::string& resourceName );

	/**
	 * Interface class of the output rendersets.
	 */
	class RenderSet : public ReferenceCount
	{
	public:
		virtual ~RenderSet(){};
		virtual bool compute() = 0;;
		virtual void save( DataSectionPtr pVisualSection, DataSectionPtr pPrimitivesSection ) = 0;
	};
	typedef SmartPointer<RenderSet> RenderSetPtr;

	static void copySections( DataSectionPtr pDest, DataSectionPtr pSrc );
private:

	uint32		nodeLimit_;
	std::string resourceName_;
	std::vector< RenderSetPtr > pRenderSets_;
	DataSectionPtr pSection_;
	NodePtr		pNode_;

	void readRenderSet( DataSectionPtr pRenderSet, DataSectionPtr pPrimitives );

	VisualSplitter( const VisualSplitter& );
	VisualSplitter& operator=( const VisualSplitter& );
};

}

#endif // VISUAL_SPLITTER_HPP
