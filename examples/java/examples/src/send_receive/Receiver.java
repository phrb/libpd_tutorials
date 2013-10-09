package send_receive;

import org.puredata.core.utils.PdDispatcher;

public class Receiver extends PdDispatcher
{
	@Override
	public void print ( String arg0 ) 
	{
		System.out.println ( arg0 );
	}
	@Override
	public void receiveBang ( String arg0 )
	{
		System.out.println ( "Banged:" + arg0 );
	}
	@Override
	public void receiveFloat ( String arg0, float arg1 )
	{
		System.out.println ( arg0 + "=" + arg1 );
	}
}
