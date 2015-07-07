#include "stdafx.h"
#include "pubnubtools.h"
#include <thread>
#include "preferences.h"
#include <mutex>


PubNubTools::PubNubTools()
{
	class PubNub;

	pubkey = cfg_publishkey;
	subkey = cfg_subscribekey;
	channel = cfg_channel;
	origin = cfg_origin;

	cm = new playlist_callback_manager;
	pcm->register_callback(cm, play_callback::flag_on_playback_all, false);
	cm->setKeys(pubkey, subkey, origin, channel);
}


PubNubTools::~PubNubTools()
{
	//static_api_ptr_t<play_callback_manager>()->unregister_callback(new service_impl_t<playlist_callback_manager>);
	//pcm->unregister_callback(new service_impl_t<playlist_callback_manager>);
	//pcm->unregister_callback(cm);

}

/*
void PubNubTools::init()
{
	//pull these from the configuration settings
	class PubNub;

	pubkey = cfg_publishkey;
	subkey = cfg_subscribekey;
	channel = cfg_channel;
	origin = cfg_origin;

}
*/

//Simple method to publish to a channel..must be subscribed
int PubNubTools::sendPublish(struct json_object* msg)
{
	pubnub_sync *sync = pubnub_sync_init();

	PubNub pn(pubkey, subkey, &pubnub_sync_callbacks, sync);
	pn.set_origin(origin);

	pn.publish(channel, *msg, -1, NULL);

	PubNub_sync_reply publish_reply = pubnub_sync_last_reply(sync);

	if (publish_reply.result() != PNR_OK)
		return 0;
	else
		return 1;

}

//Fire the thread in process memory space..static
/*
static DWORD WINAPI StaticThreadStart(LPVOID Param)
{
	PubNubTools* pn = reinterpret_cast<PubNubTools*>(Param);

	pn->ThreadStart();

	return 0;
}

//Fire the thread in process memory space....non static
void PubNubTools::ThreadStart()
{
	pubnub_sync *sync = pubnub_sync_init();
	PubNub pn(pubkey, subkey, &pubnub_sync_callbacks, sync);
}
*/

void PubNubTools::PubNubSubscribe()
{
	pubnub_sync *sync = pubnub_sync_init();
	PubNub pn(pubkey, subkey, &pubnub_sync_callbacks, sync);

	json_object * msg = json_object_new_object();
	json_object * status = json_object_new_object();

	std::string query;
	std::string searchMsg = "SEARCH_QUERY";
	std::string requestMsg = "REQUEST";
	std::string volumeMsg = "VOLUME";
	std::string trackMsg = "TRACK";
	std::string playingMsg = "GET_TRACK";
	std::string playControlMsg = "CONTROL";
	std::string playlistControlMsg = "GET_PLAYLIST";
	std::string playlistPlayMsg = "QUEUE_TRACK";
	std::string playlistRemoveMsg = "REMOVE_TRACK";
	std::string currentTrack = "";

	//Send subscribe message
	json_object_object_add(msg, "SUBSCRIBE", json_object_new_string("FooBar Subscribing to channel"));
	pn.publish(channel, *msg);

	//Send currently playing track, if playing
	status = HandlePlayingQuery();
	pn.publish(channel, *status);

	//handle messages - 
	// SEARCH_QUERY : <Search_Query_String> -> LIBRARY : <[Array of matching files]>
	// REQUEST : <File path from library> -> SUCCESS/ERROR : <Message>
	// VOLUME : <UP / DOWN/ MUTE> -> SUCCESS/ERROR : <Message>
	// GET_TRACK : <Get Current Track> -> CURRENT_TRACK : <Track name>
	// GET_PLAYLIST : <> -> PLAYLIST : <Tracks>
	// PLAY_CONTROL : <Play / Stop / Skip left / Skip right> -> SUCCESS/ERROR : <Message>
	// QUEUE_TRACK : <Playlist track> -> SUCCESS/ERROR : <Message>

	do {
		pn.subscribe(channel);

		PubNub_sync_reply subscribe_reply = pubnub_sync_last_reply(sync);

		if (subscribe_reply.result() != PNR_OK)
			console::print("Result error");

		msg = subscribe_reply.response();

		if (json_object_object_length(msg) == 0)
		{
			console::print("pubnub subscribe ok, no news");
			continue;
		}

		query = json_object_get_string(msg);

		//SEARCH_QUERY
		if (query.find(searchMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();
			std::string subQ;

			console::print("Search library for:");
			subQ = splitQuery(query);
			console::print(subQ.c_str());

			resp = HandleSearchQuery(subQ);

			pn.publish(channel, *resp);

		}
		//REQUEST
		else if (query.find(requestMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();
			std::string subQ;

			console::print("Request received for:");
			subQ = splitQuery(query);
			console::print(subQ.c_str());

			resp = HandleRequestQuery(splitQuery(query).c_str());

			pn.publish(channel, *resp);
		}
		//VOLUME up/down
		else if (query.find(volumeMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();

			console::print(query.c_str());
			//console::print("Volume control message:");

			resp = HandleVolumeQuery(splitQuery(query).c_str());

			pn.publish(channel, *resp);
		}
		//GET_TRACK
		else if (query.find(playingMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();

			console::print(query.c_str());
			//console::print("Get current track message:");

			resp = HandlePlayingQuery();

			pn.publish(channel, *resp);
		}
		//GET_PLAYLIST
		else if (query.find(playlistControlMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();

			console::print(query.c_str());
			//console::print("Get current playlist message:");

			resp = HandlePlaylistQuery();

			pn.publish(channel, *resp);
		}
		//PLAY_CONTROL
		else if (query.find(playControlMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();

			console::print(query.c_str());
			//console::print("Play control message:");

			resp = HandlePlayControlQuery(splitQuery(query).c_str());

			pn.publish(channel, *resp);
		}
		//QUEUE_TRACK control
		else if (query.find(playlistPlayMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();
			std::string subQ;

			console::print("Playing track:");
			subQ = splitQuery(query);
			console::print(subQ.c_str());

			resp = HandlePlayQuery(splitQuery(query).c_str());

			pn.publish(channel, *resp);
		}
		//REMOVE_TRACK control
		else if (query.find(playlistRemoveMsg) != std::string::npos)
		{
			json_object * resp = json_object_new_object();
			std::string subQ;

			console::print("Removing track:");
			subQ = splitQuery(query);
			console::print(subQ.c_str());

			resp = HandleRemoveQuery(splitQuery(query).c_str());

			pn.publish(channel, *resp);
		}
		//Send currently playing track if it has changed...
		else
		{
			//console::print("Unsupported query:");
			//console::print(query.c_str());
		}

		Sleep(1);

	} while (1);

	//return EXIT_SUCCESS;

}

std::string PubNubTools::splitQuery(std::string query)
{
	//Incoming JSON format - {"STRING1":"STRING2"}
	//Return query - STRING2

	std::size_t found;
	std::string subQ = "";

	found = query.find(":");
	subQ = query.substr(found, query.size() - 1);
	found = subQ.find_first_of("\"");
	subQ = subQ.substr(found + 1, subQ.find_last_of("\"") - 3);

	return subQ;
}

//Handle REQUEST from app
json_object *PubNubTools::HandleRequestQuery(std::string request)
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	int result = 0;

	//playlist manager main thread callback
	service_ptr_t<playlist_manager_update_callback> cb = new service_impl_t<playlist_manager_update_callback>();

	cb->setRequestString(request);
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);

	//get result code from callback
	result = cb->get_result();

	switch (result)
	{
		case 0:
		{
				  jstring = json_object_new_string("Request Successful");
				  json_object_object_add(jobj, "SUCCESS", jstring);
		}
			break;
		case 1:
		{
				  jstring = json_object_new_string("File not found!");
				  json_object_object_add(jobj, "ERROR", jstring);
		}
			break;
		case 2:
		{
				  jstring = json_object_new_string("Unknown error!");
				  json_object_object_add(jobj, "UNKNOWN_FAILURE", jstring);
		}
			break;
		default:
		{
				   jstring = json_object_new_string("Unknown error!");
				   json_object_object_add(jobj, "UNKNOWN_FAILURE", jstring);
		}
	}

	return jobj;
}

//Handle VOLUME query from app..input query should be UP, DOWN, or MUTE...anything else is ignored..returns JSON response 
json_object *PubNubTools::HandleVolumeQuery(std::string query)
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	int action = -1;

	if (query.find("UP") != std::string::npos)
	{
		action = 0;
	}
	else if (query.find("DOWN") != std::string::npos)
	{
		action = 1;
	}
	else if (query.find("MUTE") != std::string::npos)
	{
		action = 2;
	}
	else
	{
		action = -1;
	}

	//playback control main thread callback
	service_ptr_t<playback_control_update_callback> cb = new service_impl_t<playback_control_update_callback>();

	cb->setAction(action);
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);

	//get reslult code from callback
	action = cb->get_result();

	if (action == 0)
	{
		jstring = json_object_new_string("Volume change successful");
		json_object_object_add(jobj, "SUCCESS", jstring);
	}
	else
	{
		jstring = json_object_new_string("Error changing volume");
		json_object_object_add(jobj, "ERROR", jstring);
	}

	return jobj;

}

//Return currently playing track, if applicable
json_object *PubNubTools::HandlePlayingQuery()
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	std::string track;

	//playback control main thread callback
	service_ptr_t<playback_control_current_track_callback> cb = new service_impl_t<playback_control_current_track_callback>();

	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);

	//get result code from callback
	track = cb->get_playing();

	if (track.length() > 1)
	{
		jstring = json_object_new_string(track.c_str());
		json_object_object_add(jobj, "CURRENT_TRACK", jstring);
	}
	else
	{
		jstring = json_object_new_string("Nothing Playing!");
		json_object_object_add(jobj, "CURRENT_TRACK", jstring);
	}

	return jobj;
}

//return the entire contents of the playlist
json_object *PubNubTools::HandlePlaylistQuery()
{
	json_object * jobj = json_object_new_object();
	json_object * jarray = json_object_new_array();

	service_ptr_t<playlist_manager_contents_callback> cb = new service_impl_t<playlist_manager_contents_callback>();
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);
	pfc::list_t<metadb_handle_ptr> items = cb->get_playlist();
	int result = cb->get_result();

	if (result == 1)
	{
		t_size len = items.get_count();
		pfc::string8 track = "";

		//iterate through the entire playlist and add to the json array
		for (int i = 0; i < len; i++){
			items.get_item(i)->format_title_legacy(NULL, track, "%artist% - %title%", NULL);
			json_object *jstring = json_object_new_string(track.get_ptr());
			json_object_array_add(jarray, jstring);
		}

		json_object_object_add(jobj, "PLAYLIST", jarray);
	}
	else
	{
		json_object * jstring = json_object_new_string("Error retrieving values from active playlist!");
		json_object_object_add(jobj, "ERROR", jstring);
	}

	return jobj;
}

//Handle play control events
json_object *PubNubTools::HandlePlayControlQuery(std::string query)
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	int action = -1;

	if (query.find("PLAY") != std::string::npos)
	{
		action = 3;
	}
	else if (query.find("STOP") != std::string::npos)
	{
		action = 4;
	}
	else if (query.find("PAUSE") != std::string::npos)
	{
		action = 5;
	}
	else if (query.find("SKIP_LEFT") != std::string::npos)
	{
		action = 6;
	}
	else if (query.find("SKIP_RIGHT") != std::string::npos)
	{
		action = 7;
	}
	else
	{
		action = -1;
	}

	//playback control main thread callback
	service_ptr_t<playback_control_update_callback> cb = new service_impl_t<playback_control_update_callback>();

	cb->setAction(action);
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(5);

	//get reslult code from callback
	action = cb->get_result();

	if (action == 0)
	{
		jstring = json_object_new_string("Playback command updated successfully.");
		json_object_object_add(jobj, "SUCCESS", jstring);
	}
	else
	{
		jstring = json_object_new_string("Error issuing playback command.");
		json_object_object_add(jobj, "ERROR", jstring);
	}

	return jobj;

}

json_object *PubNubTools::HandlePlayQuery(std::string track)
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	int result = 0;

	//playlist manager main thread callback
	service_ptr_t<playlist_manager_play_callback> cb = new service_impl_t<playlist_manager_play_callback>();

	cb->setTrackString(track);
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);

	//get reslult code from callback
	result = cb->get_result();

	switch (result)
	{
	case 0:
	{
			  jstring = json_object_new_string("Playback Successful");
			  json_object_object_add(jobj, "SUCCESS", jstring);
	}
		break;
	default:
	{
			   jstring = json_object_new_string("Track does not exist in playlist!");
			   json_object_object_add(jobj, "ERROR", jstring);
	}
	}


	return jobj;
}

json_object *PubNubTools::HandleRemoveQuery(std::string track)
{
	json_object * jobj = json_object_new_object();
	json_object * jstring;

	int result = 0;

	//playlist manager main thread callback
	service_ptr_t<playlist_manager_remove_callback> cb = new service_impl_t<playlist_manager_remove_callback>();

	cb->setTrackString(track);
	static_api_ptr_t<main_thread_callback_manager> cb_manager;
	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(2);

	//get result code from callback
	result = cb->get_result();

	switch (result)
	{
	case 0:
	{
			  jstring = json_object_new_string("Track removed");
			  json_object_object_add(jobj, "SUCCESS", jstring);
	}
		break;
	default:
	{
			   jstring = json_object_new_string("Track does not exist in playlist!");
			   json_object_object_add(jobj, "ERROR", jstring);
	}
	}


	return jobj;
}

//Handle SEARCH_QUERY query from app..returns array of results or empty set
//todo: Write own std::string tolower function so there is no cycles wasted on conversion
json_object *PubNubTools::HandleSearchQuery(std::string query)
{
	//search library for the query...return array of results
	json_object * jobj = json_object_new_object();
	json_object *jarray = json_object_new_array();
	bool libStatus = false;

	pfc::list_t<metadb_handle_ptr, pfc::alloc_fast_aggressive> library;
	service_ptr_t<library_manager_search_callback> cb = new service_impl_t<library_manager_search_callback>();
	static_api_ptr_t<main_thread_callback_manager> cb_manager;

	cb_manager->add_callback(cb);

	//sleep a couple ticks to let the callback finish
	Sleep(5);
	
	libStatus = cb->checkLibrary();

	//is library enabled?
	if (!libStatus)
	{
		json_object_object_add(jobj, "LIBRARY", jarray);
		return jobj;

	}
	
	library = cb->getLibrary();

	t_size n = library.get_count();
	
	if (n < 0)
	{
		//no files exist
		json_object * jstring = json_object_new_string("Library is empty!");
		json_object_object_add(jobj, "ERROR", jstring);
		return jobj;
	}
	
	int count = 0;

	for (int i = 0; i < n; i++)
	{
		const char * cPath = library[i]->get_path();
		std::string path(cPath);

		//Full path in pfc string format..then all to lowercase
		pfc::string fPath(path.c_str());
		fPath = fPath.toLower();

		//Search query in pfc string format..then all to lowercase
		pfc::string fQuery(query.c_str());
		fQuery = fQuery.toLower();

		//Back to std::String
		path = fPath.ptr();
		query = fQuery.ptr();
		
		if (path.find(query) != std::string::npos)
		{
			json_object *jstring = json_object_new_string(library[i]->get_path());
			json_object_array_add(jarray, jstring);
			count++;

			//if count exceeds 512 then there are too many search results..return error
			if (count > 512)
			{
				json_object_put(jobj); //free jobj
				json_object * njobj = json_object_new_object();
				json_object * jstring = json_object_new_string("Search results too large!");
				json_object_object_add(njobj, "ERROR", jstring);
				return njobj;
			}
		}
		
	}
					
	json_object_object_add(jobj, "LIBRARY", jarray);
	return jobj;
	
}

/*
json_object *PubNubTools::HandleCurrentPlayingQuery(std::string &lastTrack)
{
json_object * jobj = json_object_new_object();
json_object * jstring;

std::string track;

//static play_callback_static_factory_t<playlist_callback_manager> playlist_callback_factory;

//playback control main thread callback
service_ptr_t<playback_control_current_track_callback> cb = new service_impl_t<playback_control_current_track_callback>();

static_api_ptr_t<main_thread_callback_manager> cb_manager;
cb_manager->add_callback(cb);

//sleep a couple ticks to let the callback finish
Sleep(2);

//get result code from callback
track = cb->get_playing();

if (track.length() > 1)
{
console::print("valid track");
//check to see if the current track is the same as the new track
//if so return empty jobj
if (lastTrack.compare(track))
{
console::print("Same track");
return jobj;
}
else
{
console::print("New track");
jstring = json_object_new_string(track.c_str());
json_object_object_add(jobj, "CURRENT_TRACK", jstring);
lastTrack = track;
}
}
//zero length track
else
{
console::print("No track playing");
if (lastTrack.compare("Nothing Playing!"))
{
return jobj;
}
else
{
jstring = json_object_new_string("Nothing Playing!");
json_object_object_add(jobj, "CURRENT_TRACK", jstring);
lastTrack = "Nothing Playing!";
}

}
return jobj;
}
*/


/*
void PubNubTools::RunPubNubSender(){
//pubnub_sync *sync = pubnub_sync_init();
//json_object *msg;

json_object * jobj = json_object_new_object();
json_object *jarray = json_object_new_array();

pfc::list_t<metadb_handle_ptr> library;
//metadb_handle_ptr lm;

static_api_ptr_t<library_manager> lm;
lm->get_all_items(library);

t_size n = library.get_count();
bit_array_bittable deleteMask(n);

for (int i = 0; i < n; i++){
//const file_info * fileInfo;
//console::print(library[i]->get_path());
json_object *jstring = json_object_new_string(library[i]->get_path());
json_object_array_add(jarray, jstring);
}


json_object_object_add(jobj, "Library", jarray);
//json_object_object_add(msg, "Hello", json_object_new_string("Hello From Visual C++"));
//json_object_object_add(msg, "Hello", json_object_new_array());
//pn.publish("test_channel", *jobj, -1, NULL);
//json_object_put(msg);

if (sendPublish(jobj))
{
console::print("pubnub publish ok: \n");
}

json_object_put(jobj);

}
*/