package hello_world;

import org.puredata.core.*;
import java.io.IOException;

public class Hello_World
{
	private static int[ ] patch_list = new int[ 20 ];
	private static PdReceiver receiver;

	public static void init ( )
	{
		PdBase.openAudio ( 2, 3, 44100 );
		PdBase.computeAudio ( true );
		receiver = new Receiver ( );
		PdBase.setReceiver( receiver );
	}
	public static void loadPatch ( String patch_name, int patch_ID ) throws IOException
	{
		patch_list[ patch_ID ] = PdBase.openPatch ( patch_name ); 		
	}
	public static void closePatch ( )
	{
		PdBase.release ( );
	}
	public static void main ( String [] args ) throws IOException
	{
		float[ ] in_buffer = new float[ 256 ];
		float[ ] out_buffer = new float[ 768 ];
		for (int i = 0; i < 256; i++) 
		{
			in_buffer[ i ] = i;
		}
		init ( );
		loadPatch ( "res/hello_world.pd", 0 );
		PdBase.process ( 1, in_buffer, out_buffer );
		PdBase.pollPdMessageQueue ( );
		closePatch ( );
	}
}
