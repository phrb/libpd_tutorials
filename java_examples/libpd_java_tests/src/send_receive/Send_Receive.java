package send_receive;

import org.puredata.core.*;

import java.io.IOException;

public class Send_Receive
{
	private static int patch;
	
	public static void loadPatch ( ) throws IOException
	{
		/*
		 * 
		 *  Simple initialization of libpd Java API.
		 * 
		 */
		PdBase.openAudio ( 2, 3, 44100 );
		patch = PdBase.openPatch ( "src/send_receive/send_receive_test.pd"); 
		PdBase.computeAudio ( true );
	}
	public static void closePatch ( )
	{
		PdBase.release ( );
	}
	public static void main ( String [] args ) throws IOException
	{
		PdReceiver receiver = new Receiver ();
		float[ ] in_buffer = new float[ 256 ];
		float[ ] out_buffer = new float[ 768 ];
		for (int i = 0; i < 256; i++) 
		{
			in_buffer[ i ] = i;
		}
		loadPatch ( );
		PdBase.setReceiver( receiver );
		PdBase.subscribe( "receive_bang" );
		PdBase.sendBang( "receive_bang" );
		PdBase.sendBang( "ask_hello_world" );
		PdBase.process ( 2, in_buffer, out_buffer );
		PdBase.pollPdMessageQueue ( );
		System.out.println( "Patch ID="+ patch );
		closePatch ( );
	}
}