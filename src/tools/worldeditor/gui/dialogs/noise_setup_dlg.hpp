/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NOISE_SETUP_DLG_HPP
#define NOISE_SETUP_DLG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/gui/controls/limit_slider.hpp"
#include "worldeditor/resource.h"
#include "controls/image_control.hpp"
#include "controls/edit_numeric.hpp"
#include "controls/auto_tooltip.hpp"
#include "math/simplex_noise.hpp"
#include <vector>


class NoiseSetupDlg : public CDialog
{
public:
    enum { IDD = IDD_NOISE_SETUP };

    typedef SimplexNoise::Octave Octave;

    explicit NoiseSetupDlg(CWnd *parent = NULL);

    SimplexNoise const &simplexNoise() const;
    void simplexNoise(SimplexNoise const &noise);

    std::vector<Octave> const &octaves() const;
    void octaves(std::vector<Octave> const &oct);

	float minSaturate() const;
	void minSaturate(float ms);

	float maxSaturate() const;
	void maxSaturate(float ms);

	float minStrength() const;
	void minStrength(float strength);

	float maxStrength() const;
	void maxStrength(float strength);

	static void defaultNoise( SimplexNoise & noise );

protected:
    /*virtual*/ BOOL OnInitDialog();
    /*virtual*/ void DoDataExchange(CDataExchange *pDX);

    afx_msg void OnHScroll(UINT sBCode, UINT pos, CScrollBar *scrollBar);
    afx_msg void OnOctaveEdit();

    DECLARE_MESSAGE_MAP()
    DECLARE_AUTO_TOOLTIP(NoiseSetupDlg, CDialog)

    void updateNoiseImage();

private:   
    SimplexNoise                noise_;
    controls::ImageControl8     imageControl_;
    std::vector<Octave>         octaves_;
	float						minSat_;
	float						maxSat_;
	float						minStrength_;
	float						maxStrength_;

    LimitSlider                 waveLenSlider_[8];
    controls::EditNumeric       waveLenEdit_  [8];

    LimitSlider                 weightSlider_[8];
    controls::EditNumeric       weightEdit_  [8];

    controls::EditNumeric       seedEdit_[8];

    LimitSlider                 rangeMinSlider_;
    LimitSlider                 rangeMaxSlider_;

    controls::EditNumeric       rangeMinEdit_;
    controls::EditNumeric       rangeMaxEdit_;

    LimitSlider                 satMinSlider_;
    LimitSlider                 satMaxSlider_;

    controls::EditNumeric       satMinEdit_;
    controls::EditNumeric       satMaxEdit_;

    size_t                      filter_;
    bool                        inited_;
};


#endif // NOISE_SETUP_DLG_HPP
