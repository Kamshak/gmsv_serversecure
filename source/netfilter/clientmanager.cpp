#include <netfilter/clientmanager.hpp>
#include <main.hpp>

namespace netfilter
{
	ClientManager::ClientManager( ) :
		enabled( false ), global_count( 0 ), global_last_reset( 0 ), max_window( 60 ),
		max_sec( 1 ), global_max_sec( 50 )
	{ }

	void ClientManager::SetState( bool e )
	{
		enabled = e;
	}

	bool ClientManager::CheckIPRate( uint32_t from, uint32_t time )
	{
		if( !enabled )
			return true;

		if( clients.size( ) >= MaxClients )
			for( std::map<uint32_t, Client>::iterator it = clients.begin( ); it != clients.end( ); ++it )
			{
				const Client &client = ( *it ).second;
				if( client.TimedOut( time ) && client.GetAddress( ) != from )
				{
					clients.erase( it );

					if( clients.size( ) <= PruneAmount )
						break;
				}
			}

		std::map<uint32_t, Client>::iterator it = clients.find( from );
		if( it != clients.end( ) )
		{
			Client &client = ( *it ).second;
			if( !client.CheckIPRate( time ) )
				return false;
		}
		else
			clients.insert( std::make_pair( from, Client( *this, from, time ) ) );

		if( time - global_last_reset > max_window )
		{
			global_last_reset = time;
			global_count = 1;
		}
		else
		{
			++global_count;
			if( global_count / max_window >= global_max_sec )
			{
				DebugWarning(
					"[ServerSecure] %d.%d.%d.%d reached the global query limit!\n",
					( ip >> 24 ) & 0xFF,
					( ip >> 16 ) & 0xFF,
					( ip >> 8 ) & 0xFF,
					ip & 0xFF
				);
				return false;
			}
		}

		return true;
	}

	uint32_t ClientManager::GetMaxQueriesWindow( ) const
	{
		return max_window;
	}

	uint32_t ClientManager::GetMaxQueriesPerSecond( ) const
	{
		return max_sec;
	}

	uint32_t ClientManager::GetGlobalMaxQueriesPerSecond( ) const
	{
		return global_max_sec;
	}

	void ClientManager::SetMaxQueriesWindow( uint32_t window )
	{
		max_window = window;
	}

	void ClientManager::SetMaxQueriesPerSecond( uint32_t max )
	{
		max_sec = max;
	}

	void ClientManager::SetGlobalMaxQueriesPerSecond( uint32_t max )
	{
		global_max_sec = max;
	}
}
