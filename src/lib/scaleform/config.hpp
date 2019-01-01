/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_CONFIG_HPP
#define SCALEFORM_CONFIG_HPP

// Set this define to 1 if you wish to use the Scaleform library in your game.
// Please see http://www.scaleform.com for licensing details.
#if (!defined(INDIE)) && (!defined(BW_EVALUATION))
#define SCALEFORM_SUPPORT 0
#endif

// Set this define to 1 if you wish to use the Scaleform IME library in your game.
// This comes as an additional package that must be installed over the top of
// the basic GFx installation.
// Please see http://www.scaleform.com for licensing details.

// Note that enabling Scaleform IME will disable the built-in IME support in BigWorld.
#define SCALEFORM_IME 0

#endif // SCALEFORM_CONFIG_HPP
