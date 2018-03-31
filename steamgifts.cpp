#include<iostream>
#include<vector>
#include<string>
#include<list>
#include<sstream>
#include<regex>
#include<thread>
#include<chrono>
#include<experimental/random>
#include<curlpp/cURLpp.hpp>
#include<curlpp/Easy.hpp>
#include<curlpp/Options.hpp>
#include<myhtml/api.h>

using namespace std;
using namespace curlpp::options;
using namespace std::experimental;
using namespace std::chrono;

string cookie;

struct giveaway {
	string link;
	string id;
	int copies = 1;
	int points;
};

int convertStringToInt(string &s) {
	stringstream str(s);
	int x = 0;
	if(!(str >> x))
		throw curlpp::RuntimeError("Unable to parse string " + s + " to int!");
	
	return x;
}

string parseLinkToId(string s) {
	regex reg("/giveaway/[^]{5}");
	smatch m;
	regex_search(s, m, reg);
	string prsed = "";
	if(!m.empty()) {
		prsed = m.str().substr(10, m.str().size() - 2);
	}

	return prsed;
}

int getPoints(myhtml_tree_t* tree) {
	string strnum;

	char srch_key[] = "class";
	char srch_val[] = "nav__points";
	myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, srch_key, strlen(srch_key), srch_val, strlen(srch_val), NULL);

	if(collection) {
        	for(size_t i = 0; i < collection->length; i++) {
			const char* node_text = myhtml_node_text(myhtml_node_child(collection->list[i]), NULL);
			strnum = node_text;
		}
	}

	return convertStringToInt(strnum);
}

/*
myhtml_collection_t* getGiveaways(myhtml_tree_t* tree) {
	char srch_key[] = "class";
	char srch_val[] = "giveaway__row-outer-wrap";

	return myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, srch_key, strlen(srch_key), srch_val, strlen(srch_val), NULL);
}
*/

myhtml_collection_t* getGiveawayNodes(myhtml_tree_t* tree) {
	char srch_key[] = "class";
	char srch_val[] = "giveaway__row-inner-wrap";

	myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, srch_key, strlen(srch_key), srch_val, strlen(srch_val), NULL);
	return collection;
}

vector<giveaway> getGiveaways(myhtml_tree_t* tree, myhtml_collection_t* giveawaycollection) {
	vector<giveaway> giveawaylist;

	char srch_key[] = "class";
	char srch_val[] = "giveaway__heading__name";

	for(size_t g = 0; g < giveawaycollection->length; g++) {
		myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value(tree, NULL, giveawaycollection->list[g], true, srch_key, strlen(srch_key), srch_val, strlen(srch_val), NULL);
		if(collection) {
			for(size_t i = 0; i < collection->length; i++) {
				char attr_key[] = "href";
				myhtml_tree_attr_t* attribute = myhtml_attribute_by_key(collection->list[i], attr_key, strlen(attr_key));
				mycore_string_t* attribute_string = myhtml_attribute_value_string(attribute);
				giveaway give;
				string lnk = mycore_string_data(attribute_string);
				give.link = lnk;
				give.id = parseLinkToId(lnk);
				myhtml_tree_node_t* parentNode = myhtml_node_parent(collection->list[i]);
				myhtml_collection_t* spanNodes = myhtml_get_nodes_by_tag_id_in_scope(tree, NULL, parentNode, MyHTML_TAG_SPAN, NULL);
				for(size_t node_i = 0; node_i < spanNodes->length; node_i++) {
					if(spanNodes->list[node_i]) {
						myhtml_tree_node_t* child = myhtml_node_child(spanNodes->list[node_i]);
						if(child) {
							const char* childText = myhtml_node_text(child, NULL);
							if(childText) {
								string str = childText;
								regex reg("\\([0-9]*P");
								smatch m;
								regex_search(str, m, reg);
								if(!m.empty()) {
									string m_s = m.str();
									string pnts = m_s.substr(1, m_s.size() - 2);
									give.points = convertStringToInt(pnts);
								} else {
									regex regC("\\([0-9]* C");
									smatch mC;
									regex_search(str, mC, regC);
									if(!mC.empty()) {
										string mC_s = mC.str();
										string cpies = mC_s.substr(1, mC_s.size() - 3);
										give.copies = convertStringToInt(cpies);
									}
								}
							}
						}
					
					}
				}
				giveawaylist.push_back(give);
			}
		}
	}

	return giveawaylist;
}

string getXsrfToken(myhtml_tree_t* tree) {
	char* token;

	char srch_key[] = "name";
	char srch_val[] = "xsrf_token";
	
	myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, srch_key, strlen(srch_key), srch_val, strlen(srch_val), NULL);
	if(collection) {
		char attr_key[] = "value";
		myhtml_tree_attr_t* attribute = myhtml_attribute_by_key(collection->list[0], attr_key, strlen(attr_key));
		mycore_string_t* attribute_string = myhtml_attribute_value_string(attribute);
		token = mycore_string_data(attribute_string);
	}

	return token;
}

list<string> getHeaderList(string referer) {
	list<string> headerList;
	
	string host = "www.steamgifts.com";
	string userAgent = "Mozilla/5.0 (X11; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0";
	//string accept = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
	string accept = "Accept: application/json, text/javascript, */*; q=0.01";
	string acceptLang = "en-US,en;q=0.5";
	string acceptEncoding = "gzip, deflate, br";
	//string referer = "";
	string connection = "keep-alive";
	string upgradeInsecureRequests = "1";
	string cacheControl = "max-age=0";
	
	headerList.push_back("Host: " + host);
	headerList.push_back("User-Agent: " + userAgent);
	//headerList.push_back("Accept: " + accept);
	//headerList.push_back("Accept-Language: " + acceptLang);
	//headerList.push_back("Accept-Encoding: " + acceptEncoding);
	if(!referer.empty()) {
		headerList.push_back("Referer: " + referer);
	}
	headerList.push_back("Cookie: PHPSESSID=" + cookie);
	//headerList.push_back("Connection: " + connection);
	//headerList.push_back("Upgrade-Insecure-Requests: " + upgradeInsecureRequests);
	//headerList.push_back("Cache-Control: " + cacheControl);

	return headerList;
}

myhtml_tree_t* request(string url, string postFields, string referer) {
	curlpp::Cleanup myCleanup;
	curlpp::Easy myRequest;
		
	myRequest.setOpt<Url>(url);
	myRequest.setOpt<Verbose>(true);
	myRequest.setOpt<HttpHeader>(getHeaderList(referer));

	if(!postFields.empty()) {
		myRequest.setOpt<PostFields>(postFields);
		myRequest.setOpt<PostFieldSize>(postFields.size());
	}

	stringstream strstream;
	myRequest.setOpt<WriteStream>(&strstream);
	myRequest.perform();
	
	char html[strstream.str().length() + 1];
	strcpy(html, strstream.str().c_str());
	
	// basic init
    	myhtml_t* myhtml = myhtml_create();
	myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
	
	myhtml_tree_t* tree;
	try {
    		// first tree init
    		tree = myhtml_tree_create();
    		myhtml_tree_init(tree, myhtml);
    
    		// parse html
    		myhtml_parse(tree, MyENCODING_UTF_8, html, strlen(html));
		myhtml_destroy(myhtml);
	} catch(curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
	} catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}
	
	return tree;
}

int parsePoints(string str) {
	int points;

	cout << str << ", ";
	regex reg("\"points\":\"[0-9]*\"");
	smatch m;
	regex_search(str, m, reg);
	if(!m.empty()) {
		string m_s = m.str();
		string pnts = m_s.substr(10, m_s.size() - 4);
		points = convertStringToInt(pnts);
	}

	cout << points << endl;

	return points;
}

int parsePostTree(myhtml_tree_t* tree) {
	int points = 0;
	myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG__TEXT, NULL);
    
	if(collection && collection->list && collection->length) {
		myhtml_tree_node_t *text_node = collection->list[0];
		mycore_string_t *str = myhtml_node_string(text_node);
        
        	// change data
        	char *data = mycore_string_data(str);
		points = parsePoints(data);
	}

	return points;
}

int joinGiveaway(string token, giveaway give) {
	string postFields = "xsrf_token=" + token + "&do=entry_insert&code=" + give.id;
	myhtml_tree_t* tree = request("https://www.steamgifts.com/ajax.php", postFields, give.link);
	return parsePostTree(tree);
}

int joinGiveaways(string url, int page) {
	myhtml_tree_t* tree = request(url + to_string(page), "", "");
	int points = getPoints(tree);
	if(points >= 10) {
		string token = getXsrfToken(tree);

		vector<giveaway> links = getGiveaways(tree, getGiveawayNodes(tree));

		for(giveaway g : links) {
			seconds sec(randint(2,10));
			this_thread::sleep_for(sec);
			if(!g.id.empty() && !g.link.empty() && points >= g.points) {
				points = joinGiveaway(token, g);
			}
		}
	}

	myhtml_tree_destroy(tree);
	return points;
}

int main() {
	int page = 1;
	int points = joinGiveaways("https://www.steamgifts.com/giveaways/search?page=", page);
	page++;

	while(page < 3 && points >= 10) {
		seconds sec(randint(2,10));
		this_thread::sleep_for(sec);
		points = joinGiveaways("https://www.steamgifts.com/giveaways/search?page=", page);
		page++;
	}

	return 0;
}

