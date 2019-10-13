// Eyes

#include "Eyes.h"
#include "EyesView.h"

#include <Alert.h>
#include <Path.h>
#include <Roster.h>
#include <Deskbar.h>

#include <stdio.h>

int main( void)
{
	Eyes* Application = new Eyes();

	delete Application;

	return( 0);
}

Eyes::Eyes( void)
    : BApplication( APPLICATION_SIGNATURE)
{
	auto_ptr< BDeskbar> pDeskbar( new BDeskbar());

	// If already installed--remove
	if( pDeskbar->HasItem( VIEW_SIGNATURE))
	{
		status_t err = pDeskbar->RemoveItem( VIEW_SIGNATURE);

		if( err != B_OK)
		{
			( new BAlert( NULL, strerror( err), "OK"))->Go( 0);
		}
	}
	else
	{
		BPath AppDirectory;
		const char* ApplicationDirectory = NULL;

		// app directory
		app_info appinfo;
	
		if( GetAppInfo( &appinfo) == B_OK)
		{
			BEntry ApplicationEntry( &appinfo.ref);
			BEntry ApplicationDirectoryEntry;

			if( ApplicationEntry.GetParent( &ApplicationDirectoryEntry) == B_OK)
			{
				if( AppDirectory.SetTo( &ApplicationDirectoryEntry) == B_OK)
				{
					ApplicationDirectory = AppDirectory.Path();
				}
			}
		}

		// ??? MM View BRect an Bitmap angepasst
		auto_ptr< EyesView> pReplicant( new EyesView( ApplicationDirectory, BRect( 0, 0, 30, 15)));

		status_t err = pDeskbar->AddItem( pReplicant.get());

		if( err != B_OK)
		{
			( new BAlert( NULL, strerror( err), "OK"))->Go( 0);
		}
	}
}
