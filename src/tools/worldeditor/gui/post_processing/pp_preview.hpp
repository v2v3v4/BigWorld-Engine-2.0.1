/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PP_PREVIEW_HPP
#define PP_PREVIEW_HPP

#include "cstdmf/init_singleton.hpp"
#include "romp/py_render_target.hpp"
#include "post_processing/debug.hpp"

namespace PostProcessing
{
	class Phase;
};

class PhaseNodeView;
typedef SmartPointer<PhaseNodeView> PhaseNodeViewPtr;

class PPPreview : public PostProcessing::Debug
{
	Py_Header( PPPreview, Debug )
public:
	PPPreview( PyTypePlus *pType = &s_type_ );
	~PPPreview();

	void edLayout( RECT& clientRect, const CPoint& offset, std::vector<PhaseNodeView*>& pnvVector );
	bool phaseRect( const PostProcessing::Phase *, CRect& out );
	bool phaseRectForResults( const PostProcessing::Phase *, CRect& out );
	void copyResults();
	bool isVisible( PhaseNodeView* pnv ) const;

	virtual void beginChain( uint32 nEffects );
	virtual void beginEffect( uint32 nPhases );
	virtual RECT nextPhase();
	virtual Vector4 phaseUV( uint32 effect, uint32 phase, uint32 nEffects, uint32 nPhases );

	PyRenderTargetPtr pRT() const	{ return previewRT_; }
	ComObjectWrap<DX::Texture> pPreview()	{ return pSystemTex_; }

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()
private:
	typedef PPPreview This;
	PyRenderTargetPtr previewRT_;
	ComObjectWrap<DX::Texture> pSystemTex_;
	typedef std::map<const PostProcessing::Phase*, CRect> LayoutMap;
	LayoutMap layoutMap_;
	LayoutMap resultsLayoutMap_;
};

typedef SmartPointer<PPPreview> PPPreviewPtr;


#endif //#ifndef PP_PREVIEW_HPP
