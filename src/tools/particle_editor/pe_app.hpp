/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PE_APP_HPP
#define PE_APP_HPP

class PeApp
{
public:
    PeApp();

    ~PeApp();

    static PeApp &instance();

private:
    // Not permitted
    PeApp(PeApp const &);
    PeApp &operator=(PeApp const &);

private:
    static PeApp        *s_instance;
};

#endif // PE_APP_HPP
