package hello_world;

import org.puredata.core.utils.PdDispatcher;

public class Receiver extends PdDispatcher
{
	@Override
	public void print ( String arg0 ) 
	{
		System.out.println ( arg0 );
	}
}
