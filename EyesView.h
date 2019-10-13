// EyesView

#ifndef EYESVIEW_H
#define EYESVIEW_H

#include <Bitmap.h>
#include <Font.h>
#include <MessageRunner.h>
#include <PopUpMenu.h>
#include <View.h>

#include <memory>

using namespace std;

class EyesView : public BView
{
	private:
		enum
		{
			MSG_QUIT			= 'quit',
			MSG_ABOUT			= 'abut',
			MSG_PREFS			= 'pref',
			MSG_MOUSE			= 'mous'
		};

		auto_ptr< BMessageRunner> m_pMessageRunner;
		int32 m_nCurrWorkspace;
		BFont m_Font;
		auto_ptr< BPopUpMenu> m_pPopupMenu;

		BBitmap* m_LeftEyeImages;
		BBitmap* m_RightEyeImages;

		BPoint m_MousePoint;

		char* m_ApplicationDirectory;

		bool m_MouseRunning;

		void startMouseThread( void);
		void stopMouseThread( void);

		static int32 mouseThreadStub( void* Data);
		int32 mouseThread( void);

		bool loadImage( const char* FileName, BBitmap** ImageBitmap);
		void drawEye( BBitmap* EyeImages, BPoint MousePoint, BPoint EyeCenter, BPoint Where,
		              float EyesWidth, float EyesHeight);

		void Remove( void);

	public:
		EyesView( const char* ApplicationDirectory, BRect Rect);
		EyesView( BMessage* Message);
		virtual ~EyesView( void);

		virtual void AttachedToWindow( void);
		virtual void MessageReceived( BMessage* Message);
		virtual void Draw( BRect Rect);
		virtual status_t Archive( BMessage* Data, bool Deep = true) const;
		virtual void MouseDown( BPoint point);

		static EyesView* Instantiate( BMessage* Data);
};

#endif
