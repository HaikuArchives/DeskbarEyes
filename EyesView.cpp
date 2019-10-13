// EyesView

#include <Alert.h>
#include <Application.h>
#include <BitmapStream.h>
#include <Deskbar.h>
#include <File.h>
#include <Path.h>
#include <MenuItem.h>
#include <Roster.h>
#include <Screen.h>
#include <TranslatorRoster.h>

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Bitmaps.h"
#include "Eyes.h"
#include "EyesView.h"

EyesView::EyesView( const char* ApplicationDirectory, BRect Rect)
        : BView( Rect, VIEW_SIGNATURE, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	if( ApplicationDirectory)
	{
		m_ApplicationDirectory = strdup( ApplicationDirectory);
	}
	else
	{
		m_ApplicationDirectory = NULL;
	}
}

EyesView::EyesView( BMessage* Message)
        : BView( Message)
        , m_ApplicationDirectory( NULL)

{
	SetResizingMode( B_FOLLOW_ALL_SIDES);
	SetFlags( B_WILL_DRAW);

	SetDrawingMode( B_OP_OVER);

	const char* ApplicationDirectory;

	Message->FindString( "eyes:appplicationdirectory", &ApplicationDirectory);

	if( ApplicationDirectory)
	{
		m_ApplicationDirectory = strdup( ApplicationDirectory);
	}
	else
	{
		m_ApplicationDirectory = NULL;
	}	
}

EyesView::~EyesView( void)
{
	free( m_ApplicationDirectory);
}

// Instantiate EyesView from archive
EyesView* EyesView::Instantiate( BMessage* Message)
{
	if( !validate_instantiation( Message, "EyesView"))
	{
		return( NULL);
	}

	return( new EyesView( Message));
}

status_t EyesView::Archive( BMessage* Data, bool Deep) const
{
	BView::Archive( Data, Deep);

	Data->AddString( "add_on", APPLICATION_SIGNATURE);
	Data->AddString( "class", "EyesView");
	Data->AddString( "eyes:appplicationdirectory", m_ApplicationDirectory);

	return( B_OK);
}

void EyesView::AttachedToWindow( void)
{
	SetViewColor( B_TRANSPARENT_COLOR);

	// Load Bitmaps.
	m_LeftEyeImages = NULL;
	m_RightEyeImages = NULL;

	loadImage( "Left_Eyes", &m_LeftEyeImages);
	loadImage( "Right_Eyes", &m_RightEyeImages);

	// Build popup menu
	m_pPopupMenu.reset( new BPopUpMenu( "PopUpMenu", false, false));
	m_pPopupMenu->AddItem( new BMenuItem( "About...", new BMessage( MSG_ABOUT)));
	m_pPopupMenu->AddSeparatorItem();
	
	// ??? MM Menueintrag ergaenzt
	m_pPopupMenu->AddItem( new BMenuItem( "Preferences...", new BMessage( MSG_PREFS)));
	m_pPopupMenu->AddSeparatorItem();
	// ??? MM Menueintrag ergaenzt

	m_pPopupMenu->AddItem( new BMenuItem("Quit", new BMessage( MSG_QUIT)));
	m_pPopupMenu->SetTargetForItems( this);

	startMouseThread();
}

// Remove Deskbar add-on
void EyesView::Remove( void)
{
	stopMouseThread();

	auto_ptr< BDeskbar> pDeskbar( new BDeskbar());

	status_t err = pDeskbar->RemoveItem( VIEW_SIGNATURE);

	if( err != B_OK)
	{
		( new BAlert( NULL, strerror( err), "OK"))->Go( NULL);
	}
}

// Display popup menu on right mouse click
void EyesView::MouseDown( BPoint ptPoint)
{
	MakeFocus( true);

	BPoint ptCursor;
	uint32 uButtons= 0;
	GetMouse(&ptCursor, &uButtons, true);

	if( uButtons & B_SECONDARY_MOUSE_BUTTON)
	{
		ConvertToScreen( &ptPoint);
		m_pPopupMenu->Bounds().PrintToStream();
		m_pPopupMenu->Go( ptPoint, true, true, BRect( ptPoint, ptPoint + BPoint( 20, 20)), true);
	}
}

void EyesView::MessageReceived( BMessage* Message)
{
	switch( Message->what)
	{
		case MSG_MOUSE :
		{
			if( Window()->Lock())
			{
				Message->FindPoint( "Location", &m_MousePoint);

				Invalidate();

				Window()->Unlock();
			}
		}
		break;

		case MSG_QUIT:
		{
			// Remove workspace indicator from Deskbar
			Remove();
		}
		break;
		
		// ??? MM Preferences Message abfangen
		case MSG_PREFS:
		{
			// View Preferences
		}
		break;
		// ??? MM Preferences Message abfangen

		// ??? MM About Text angepasst
		case MSG_ABOUT:
		{
			// Display about app info
			( new BAlert("DeskbarEyes",
				"Who is watching you?\n\n"
				"© 2004 Peter Stegemann, Richard & Mark Erben\n\n"
				"Preferences by Markus Maier", "OK"))->Go();
		// ??? MM About Text angepasst
		}
		break;

		default:
		{
			BView::MessageReceived( Message);
		}
		break;
	}
}

// Redraw current workspace number
void EyesView::Draw( BRect Rect)
{
	// ??? MM Zeichen Aktionen ersetzt
	Rect = Bounds();

	float EyesWidth = Rect.Width() / 2;
	float EyesHeight = Rect.Height();

	BPoint LeftEyeCenter( EyesWidth / 2, EyesHeight / 2);
	BPoint RightEyeCenter( EyesWidth + EyesWidth / 2, EyesHeight / 2);

	// Coordinates of our view on the screen.
	BPoint ScreenViewPoint( 0, 0);
	ConvertToScreen( &ScreenViewPoint);

	// MousePoint in the coordinate system of our view.
	BPoint MousePoint = m_MousePoint - ScreenViewPoint;

//	m_MousePoint.PrintToStream();
//	MousePoint.PrintToStream();

	SetHighColor( Parent()->ViewColor());
	SetLowColor( Parent()->ViewColor());
	FillRect( Rect);

	SetHighColor( 0, 0, 0);

	drawEye( m_LeftEyeImages, MousePoint, LeftEyeCenter, BPoint( 0, 0), EyesWidth, EyesHeight);
	drawEye( m_RightEyeImages, MousePoint, RightEyeCenter, BPoint( EyesWidth, 0), EyesWidth, EyesHeight);
}

void EyesView::drawEye( BBitmap* EyeImages, BPoint MousePoint, BPoint EyeCenter, BPoint Where,
                        float EyesWidth, float EyesHeight)
{
	if( !EyeImages)
	{
		StrokeLine( EyeCenter, MousePoint);
	}
	else
	{
		int Index = 0;

		BRect EyeRect( Where.x, Where.y, Where.x + EyesWidth, Where.y + EyesHeight);

		if( !EyeRect.Contains( MousePoint))
		{
			BRect ImageSize = EyeImages->Bounds();

			int ImageCount = ( ImageSize.Height() + 1) / EyesHeight;

			if( ImageCount)
			{
				BPoint Difference = MousePoint - EyeCenter;

				float RadiantAngle;

				if( Difference.x == 0.0)
				{
					if( Difference.y > 0)
					{
						RadiantAngle = M_PI / 2;
					}
					else
					{
						RadiantAngle = -M_PI / 2;
					}
				}
				else
				{
					if( Difference.x > 0)
					{
						RadiantAngle = atan( Difference.y / Difference.x);
					}
					else
					{
						RadiantAngle = atan( Difference.y / -Difference.x);
					}
				}

				float FloatIndex = ( RadiantAngle / ( M_PI * 2));

				if( Difference.x > 0)
				{
					FloatIndex = 0.25 + FloatIndex;
				}
				else
				{
					FloatIndex = 0.75 - FloatIndex;
				}

				int AnimatedImages = ImageCount - 1;

				float ImageAngle = 1.0 / AnimatedImages;

				FloatIndex += ImageAngle / 2;

				// Clip 1.0 into range.
				if( FloatIndex >= 1.0)
				{
					FloatIndex -= 1.0;
				}

				Index = ( AnimatedImages * FloatIndex) + 1;

//			fprintf( stderr, "%f %d %d\n", FloatIndex, ImageCount, Index);
			}
		}

		DrawBitmap( EyeImages,
		            BRect( 0, EyesHeight * Index, EyesWidth, EyesHeight * ( Index + 1)),
		            EyeRect);
	}
}

void EyesView::startMouseThread( void)
{
	// Start mouse thread.
	thread_id ThreadID;

	if(( ThreadID = spawn_thread( mouseThreadStub, "EyesMouseThread", B_DISPLAY_PRIORITY, this)) >= B_OK)
	{
		m_MouseRunning = true;

		resume_thread( ThreadID);
	}
}

void EyesView::stopMouseThread( void)
{
	// Stop mouse thread.
	do
	{
		m_MouseRunning = false;
		
		snooze( 20000);
	}
	while( !m_MouseRunning);
}

int32 EyesView::mouseThreadStub( void* Data)
{
	EyesView* View = static_cast< EyesView*>( Data);

	return( View->mouseThread());
}

int32 EyesView::mouseThread( void)
{
	uint64 Count = 0;
	BPoint Point;

	bool PrimaryDown = false;
	bool SecondaryDown = false;
	bool TertiaryDown = false;

	BMessenger Messenger( this);

	while( m_MouseRunning)
	{
		bool MouseStateChanged = false;

		Count++;

		// Prepare message.
		BMessage Message( MSG_MOUSE);

		uint32 Buttons;
		BPoint NewPoint;

		// Get mouse state.
		if( Window()->Lock())
		{
			GetMouse( &NewPoint, &Buttons);
			ConvertToScreen( &NewPoint);

			Window()->Unlock();

			// Check mouse buttons.		
			if( Buttons & B_PRIMARY_MOUSE_BUTTON)
			{
				if( !PrimaryDown)
				{
					// Primary down.
					PrimaryDown = true;

					Message.AddInt32( "Primary", 1);
					MouseStateChanged = true;
				}
			}
			else
			{
				if( PrimaryDown)
				{
					// Primary up.
					PrimaryDown = false;

					Message.AddInt32( "Primary", 0);
					MouseStateChanged = true;
				}
			}

			if( Buttons & B_SECONDARY_MOUSE_BUTTON)
			{
				if( !SecondaryDown)
				{
					// Secondary down.
					SecondaryDown = true;

					Message.AddInt32( "Secondary", 1);
					MouseStateChanged = true;
				}
			}
			else
			{
				if( SecondaryDown)
				{
					// Secondary up.
					SecondaryDown = false;

					Message.AddInt32( "Secondary", 0);
					MouseStateChanged = true;
				}
			}

			if( Buttons & B_TERTIARY_MOUSE_BUTTON)
			{
				if( !TertiaryDown)
				{
					// Tertiary down.
					TertiaryDown = true;

					Message.AddInt32( "Tertiary", 1);
					MouseStateChanged = true;
				}
			}
			else
			{
				if( TertiaryDown)
				{
					// Tertiary up.
					TertiaryDown = false;

					Message.AddInt32( "Tertiary", 0);
					MouseStateChanged = true;
				}
			}

			// Check mouse position only every 1/3 time.
			if( MouseStateChanged || !( Count % 3))
			{
				if( Point != NewPoint)
				{
					// If the mouse has been moved, remember new position and add event.
					Point = NewPoint;
					Message.AddInt32( "Moved", 1);

					Count = 0;
					MouseStateChanged = true;
				}
			}

			if( MouseStateChanged)
			{
				Message.AddInt64( "TimeStamp", system_time());
				Message.AddInt32( "Workspace", current_workspace());
				Message.AddPoint( "Location", NewPoint);

				Messenger.SendMessage( &Message);
			}
		}

		// Wait 1/25 seconds.
		snooze( 40000);	
	}

	m_MouseRunning = true;

	return( 0);
}

bool EyesView::loadImage( const char* FileName, BBitmap** ImageBitmap)
{
	BFile ImageFile;
	BPath ImagePath;

	// Use absolute paths directly.
	if( *FileName == '/')
	{
		ImageFile.SetTo( FileName, B_READ_ONLY);
	}
	else
	{
		// App directory.
		if( m_ApplicationDirectory)
		{
			if( ImagePath.SetTo( m_ApplicationDirectory, "Images") == B_OK)
			{
				if( ImagePath.Append( FileName) == B_OK)
				{
					ImageFile.SetTo( ImagePath.Path(), B_READ_ONLY);
				}
			}

			if( ImageFile.InitCheck() != B_OK)
			{
	 			if( ImagePath.SetTo( m_ApplicationDirectory, FileName) == B_OK)
				{
					ImageFile.SetTo( ImagePath.Path(), B_READ_ONLY);
				}
	 		}
		}
		else
		{
			fprintf( stderr, "No application directory.\n");
		}
	}

	if( ImageFile.InitCheck() != B_OK)
	{
		if( ImagePath.SetTo( "Images") == B_OK)
		{
			if( ImagePath.Append( FileName) == B_OK)
			{
				ImageFile.SetTo( ImagePath.Path(), B_READ_ONLY);
			}
		}
	}

	if( ImageFile.InitCheck() != B_OK)
	{
		ImageFile.SetTo( FileName, B_READ_ONLY);
	}

	if( ImageFile.InitCheck() != B_OK)
	{
		fprintf( stderr, "Image file %s not found.\n", FileName);

		return( false);
	}

	BTranslatorRoster* Roster = BTranslatorRoster::Default();

	if( !Roster)
	{
		fprintf( stderr, "Translator roster not found.\n");

		return( false);
	}

	BBitmapStream Stream;

	if( Roster->Translate( &ImageFile, NULL, NULL, &Stream, B_TRANSLATOR_BITMAP) < B_OK)
	{
		fprintf( stderr, "Translation of image file %s failed.\n", FileName);

		return( false);
	}

	if( Stream.DetachBitmap( ImageBitmap) != B_OK)
	{
		fprintf( stderr, "Detaching bitmap for image file %s failed.\n", FileName);

		return( false);
	}

	return( true);
}
