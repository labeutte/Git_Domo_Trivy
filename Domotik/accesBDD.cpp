#include <sstream>
#include "accesBDD.h"
/*  Version du 25/11/2020 */

#define Debug 0

/**
 * Constructeur/Destructeur
 */
accesBDD::accesBDD() {
	eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
	if (eResult != XML_SUCCESS) {
		std::cerr << "accesBDD : Erreur de chargement du fichier XML" << std::endl;
	}
	XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
	if (rootNode == nullptr) {
		puts("Pas de noeud XML Domotik");
	} else {
		element = rootNode->FirstChildElement("Bdd");
		if (element != nullptr) {
			item = element->FirstChildElement("hostBdd");
			string st = item->GetText();
			host = st;
			item = element->FirstChildElement("base");
			dbname = item->GetText();
			item = element->FirstChildElement("login");
			login = item->GetText();
			item = element->FirstChildElement("mdp");
			pwd = item->GetText();
		}
	}
	//if (Debug)
	//    cout << "BDD=" << host << " " << login << "  " << pwd << endl;
}

accesBDD::~accesBDD() {
}

/**
 *
 */
string accesBDD::dateL() {
	const char* nomJourSem[] = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
	const char* nomMois[] = {"janv", "fev", "mar", "avr", "mai", "juin", "juil", "aout", "sept", "oct", "nov", "dec"};

	char locTime[30];
	time_t TH;
	struct tm* t;
	TH = time(NULL);
	t = localtime(&TH);
	sprintf(locTime, "%3s %02u %s %04u %02u:%02u:%02u", nomJourSem[t->tm_wday], t->tm_mday, nomMois[t->tm_mon], 1900 + t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);
	return (string(locTime));
}

void accesBDD::sortErr(int nb) {
	dt = dateL();
	if (Debug)
		fprintf(stderr, "%s- Erreur BdD -%d\n", dt.c_str(), nb);
}

/**
 * 
 * @param grp
 * @return 
 */
string accesBDD::getnum(string grp) {
	MYSQL *mysql;
	string req;
	string num;
	if (grp == "1") {
		req = "SELECT Tel FROM PERSONNE WHERE Groupe=1";
	} else if (grp == "2") {
		req = "SELECT Tel FROM PERSONNE WHERE Groupe=1 OR Groupe=2";
	}

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
				num += "-";
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		return num;
	} else {
		mysql_close(mysql);
		sortErr(1);
		return ("");
	}
}

/**
 * 
 * @param id
 * @param temp
 * @return 
 */
void accesBDD::updatetemp(unsigned long id, float temp) {
	MYSQL *mysql;
	string Resultid; // string which will contain the result
	ostringstream conver; // stream used for the conversion
	conver << id; // insert the textual representation of 'Number' in the characters in the stream
	Resultid = conver.str(); // set 'Result' to the contents of the stream

	string Resulttemp; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << temp; // insert the textual representation of 'Number' in the characters in the stream
	Resulttemp = convert.str(); // set 'Result' to the contents of the stream

	string req = "UPDATE `CAPTEUR` SET `Valeur` =";
	req += Resulttemp;
	req += " WHERE `CAPTEUR`.`Code` = ";
	req += Resultid;

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(2);
	}
	mysql_close(mysql);
}

/**
 * 
 * @param id
 * @param temp
 * @return 
 */
void accesBDD::majtemp(string cod, string temp) {
	MYSQL *mysql;
	string req = "UPDATE `CAPTEUR` SET `Valeur` = ";
	req += temp;
	req += " WHERE `CAPTEUR`.`Code` = ";
	req += cod;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(20);
	}
	mysql_close(mysql);
}

/**
 * 
 * @param idcapt
 * @return 
 */
void accesBDD::alertcapt(string idcapt, string nom) {
	//INSERT INTO `ALERTE` (`IdAlert`, `DateH`, `IdType`, `IdCapt`, `IdCam`) VALUES (NULL, '2017-03-16 00:00:00', '2', '2', NULL);
	MYSQL *mysql;
	time_t now;
	char *date;
	time(&now);
	date = ctime(&now);

	string Resultdate; // string which will contain the result
	ostringstream converc; // stream used for the conversion
	converc << date; // insert the textual representation of 'Number' in the characters in the stream
	Resultdate = converc.str(); // set 'Result' to the contents of the stream

	/*string Resultid;          // string which will contain the result
	ostringstream conver;   // stream used for the conversion
	conver << idtype;      // insert the textual representation of 'Number' in the characters in the stream
	Resultid = conver.str(); // set 'Result' to the contents of the stream*/

	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	//convert << code;      // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req = "INSERT INTO `ALERTE` VALUES (NULL, '";
	req += Resultdate;
	req += "', ";
	req += idcapt;
	req += ", NULL, '";
	req += nom;
	req += "', CURRENT_DATE())";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(3);
	}
	mysql_close(mysql);
}

string accesBDD::getidcapt(int code) {
	MYSQL *mysql;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << code; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream
	string req, num;
	string idcapt;
	req = "SELECT IdCapt FROM `CAPTEUR` WHERE Code=";
	req += Resultcode;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("0");
	} else {
		mysql_close(mysql);
		sortErr(4);
		return ("0");
	}
}

string accesBDD::getTypeCapt(unsigned long code) {
	MYSQL *mysql;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << code; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream
	string req, num;
	string typeCapt;
	req = "SELECT Type FROM `CAPTEUR` WHERE Code=";
	req += Resultcode;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("0");
	} else {
		mysql_close(mysql);
		sortErr(4);
		return ("0");
	}
}

/**
 * 
 * @param code
 * @return 
 */
float accesBDD::gettemp(int code) {
	MYSQL *mysql;
	float temp;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << code; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req, num;
	string idcapt;
	req = "SELECT Valeur FROM `CAPTEUR` WHERE Code=";
	req += Resultcode;

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		//int totalrows = mysql_num_rows(result);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		temp = atof(num.c_str());
		return temp;
	} else {
		mysql_close(mysql);
		sortErr(5);
		return 0;
	}
}

string accesBDD::gettempToutes() {
	MYSQL *mysql;
	float temp;
	string Resultcode = "\n"; // string which will contain the result
	string req, num = "";
	req = "SELECT PosGeo, Valeur FROM `CAPTEUR` WHERE Code>19 AND Code<30 AND Valeur<90";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		while ((row = mysql_fetch_row(result))) {
			if (row[0] != NULL)
				Resultcode += row[0];
			else
				Resultcode += "ss position";
			Resultcode += "=";
			Resultcode += row[1];
			Resultcode += " deg\n";
		}
		mysql_free_result(result);
		mysql_close(mysql);
		return Resultcode;
	} else {
		mysql_close(mysql);
		sortErr(5);
		return 0;
	}

}

/**
 * 
 * @param mdp
 * @return 
 */
bool accesBDD::testpwd(string mdp) {
	MYSQL *mysql;
	//SELECT CASE WHEN pwd="123" then 'true' ELSE 'false' END FROM INFO
	string req, pass;
	req = "SELECT CASE WHEN pwd='";
	req += mdp;
	req += "' then 'true' ELSE 'false' END FROM INFO";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				pass += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		if (pass == "true") {
			return 1;
		} else {
			return 0;
		}
	} else {
		mysql_close(mysql);
		sortErr(6);
		return 0;
	}
}

/**
 * 
 * @param mdp
 * @return 
 */
bool accesBDD::testpwdSMS(string mdp) {
	MYSQL *mysql;
	//SELECT CASE WHEN pwd="123" then 'true' ELSE 'false' END FROM INFO
	string req, pass;
	req = "SELECT CASE WHEN pwdSMS='";
	req += mdp;
	req += "' then 'true' ELSE 'false' END FROM INFO";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				pass += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		if (pass == "true") {
			return 1;
		} else {
			return 0;
		}
	} else {
		mysql_close(mysql);
		sortErr(7);
		return 0;
	}
}

/**
 * 
 * @return 
 */
int accesBDD::getstate() {
	MYSQL *mysql;
	int i;
	string req, state;
	req = "SELECT state FROM `INFO` WHERE IdInfo=1";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		//if(mysql_real_connect(mysql,"localhost","gilles","gilles","Surveillance",0,NULL,0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				state += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		i = atoi(state.c_str());
		return i;
	} else {
		mysql_close(mysql);
		dt = dateL();
		sortErr(8);
		return 0;
	}
}

unsigned char accesBDD::testEtatRelai() {
	MYSQL *mysql;
	unsigned char et;
	string req, state;
	req = "SELECT etatVMC FROM `INFO`";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");
	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		//if(mysql_real_connect(mysql,"localhost","gilles","gilles","Surveillance",0,NULL,0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				state += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		et = atoi(state.c_str());
		return et;
	} else {
		mysql_close(mysql);
		dt = dateL();
		sortErr(8);
		return 2;
	}
}

/**
 * 
 * @return 
 */
/*
int accesBDD::getcle3G() {
	MYSQL *mysql;
	int i;
	string req, state;
	req = "SELECT cle3G FROM `INFO` WHERE IdInfo=1";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				state += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		i = atoi(state.c_str());
		return i;
	} else {
		mysql_close(mysql);
		dt = dateL();
		sortErr(80);
		return 0;
	}
}
 */
/*
int accesBDD::getemet433() {
	MYSQL *mysql;
	int i;
	string req, state;
	req = "SELECT emet433 FROM `INFO` WHERE IdInfo=1";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				state += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		i = atoi(state.c_str());
		return i;
	} else {
		mysql_close(mysql);
		dt = dateL();
		sortErr(80);
		return 0;
	}
}
 */

/**
 * 
 * @param code
 * @return 
 */
float accesBDD::gettempalert(int code) {
	MYSQL *mysql;
	float temp;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << code; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req, num;
	string idcapt;
	req = "SELECT alert FROM `CAPTEUR` WHERE Code=";
	req += Resultcode;

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		temp = atof(num.c_str());
		return temp;
	} else {
		mysql_close(mysql);
		sortErr(9);
		return 0;
	}
}

/**
 * 
 * @param IdCam
 * @return 
 */
void accesBDD::alertcam(string IdCam, string nom) {
	MYSQL *mysql;
	time_t now;
	char *date;
	time(&now);
	date = ctime(&now);

	string Resultdate; // string which will contain the result
	ostringstream converc; // stream used for the conversion
	converc << date; // insert the textual representation of 'Number' in the characters in the stream
	Resultdate = converc.str(); // set 'Result' to the contents of the stream

	string req = "INSERT INTO `ALERTE` VALUES (NULL, '";
	req += Resultdate;
	req += "', NULL, '";
	req += IdCam;
	req += "', '";
	req += nom;
	req += "', CURRENT_DATE())";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(10);
	}
	mysql_close(mysql);
}

/**
 * 
 * @param nom
 * @return 
 */
string accesBDD::getidcam(string nom) {
	MYSQL *mysql;
	string req, num;
	string idcapt;

	req = "SELECT IdCam FROM `CAMERA` WHERE NOM='";
	req += nom;
	req += "'";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("0");
	} else {
		mysql_close(mysql);
		sortErr(11);
	}
	return ("");
}

/**
 * 
 * @param newstate
 * @return 
 */
void accesBDD::updatestate(int newstate) {
	MYSQL *mysql;
	string Resultstate; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << newstate; // insert the textual representation of 'Number' in the characters in the stream
	Resultstate = convert.str(); // set 'Result' to the contents of the stream

	string req = "UPDATE `INFO` SET state =";
	req += Resultstate;

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(12);
	}
	mysql_close(mysql);
}

void accesBDD::updateEtatRelai(int newstate) {
	MYSQL *mysql;
	string Resultstate; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << newstate; // insert the textual representation of 'Number' in the characters in the stream
	Resultstate = convert.str(); // set 'Result' to the contents of the stream

	string req = "UPDATE `INFO` SET etatVMC = ";
	req += Resultstate;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");
	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(24);
	}
	mysql_close(mysql);
}

/**
 * 
 * @param code
 * @return 
 */
string accesBDD::getcaptname(int code) {
	MYSQL *mysql;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << code; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req, num;
	string idcapt;
	req = "SELECT Nom FROM `CAPTEUR` WHERE Code=";
	req += Resultcode;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("Capteur inconnu");
	} else {
		mysql_close(mysql);
		sortErr(13);
	}
	return ("");
}

/**
 * 
 * @return 
 */
void accesBDD::coupure() {
	MYSQL *mysql;
	//INSERT INTO `ALERTE` (`IdAlert`, `DateH`, `IdCapt`, `IdCam`, `description`) VALUES (NULL, 'test', NULL, NULL, 'coupure');
	time_t now;
	char *date;
	time(&now);
	date = ctime(&now);

	string Resultdate; // string which will contain the result
	ostringstream converc; // stream used for the conversion
	converc << date; // insert the textual representation of 'Number' in the characters in the stream
	Resultdate = converc.str(); // set 'Result' to the contents of the stream

	string req = "INSERT INTO `ALERTE` VALUES (NULL, '";
	req += Resultdate;
	req += "', NULL, NULL, 'Coupure de courant', CURRENT_DATE());";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		mysql_close(mysql);
		sortErr(14);
	}
}

/**
 * 
 * @return 
 */
void accesBDD::retour() {
	MYSQL *mysql;
	//INSERT INTO `ALERTE` (`IdAlert`, `DateH`, `IdCapt`, `IdCam`, `description`) VALUES (NULL, 'test', NULL, NULL, 'coupure');
	time_t now;
	char *date;
	time(&now);
	date = ctime(&now);

	string Resultdate; // string which will contain the result
	ostringstream converc; // stream used for the conversion
	converc << date; // insert the textual representation of 'Number' in the characters in the stream
	Resultdate = converc.str(); // set 'Result' to the contents of the stream

	string req = "INSERT INTO `ALERTE` VALUES (NULL, '";
	req += Resultdate;
	req += "', NULL, NULL, 'Retour du courant', CURRENT_DATE());";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");
	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(15);
	}
	mysql_close(mysql);
}

string accesBDD::getNomSite() {
	MYSQL *mysql;
	string req, resultat;
	req = "SELECT nomSite FROM `INFO` WHERE IdInfo=1";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				resultat += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		return resultat;
	} else {
		mysql_close(mysql);
		sortErr(30);
		return "";
	}
}

/**
 * 
 * @return 
 */
int accesBDD::getNbCam() {
	MYSQL *mysql;
	string req, resultat;
	req = "SELECT count(*) FROM `CAMERA`";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				resultat += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		return atoi(resultat.c_str());
	} else {
		mysql_close(mysql);
		sortErr(16);
		return 0;
	}
}

/**
 * 
 * @param id
 * @return 
 */
string accesBDD::getcamRep(string id) {
	MYSQL *mysql;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << id; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req, num;
	string idcapt;
	req = "SELECT NomRep FROM `CAMERA` WHERE ID='";
	req += id;
	req += "'";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int totalrows = mysql_num_rows(result);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("Camera inconnue");
	} else {
		mysql_close(mysql);
		sortErr(170);
	}
	return ("");
}

/**
 * 
 * @param id
 * @return 
 */
string accesBDD::getcamname(string id) {
	MYSQL *mysql;
	string Resultcode; // string which will contain the result
	ostringstream convert; // stream used for the conversion
	convert << id; // insert the textual representation of 'Number' in the characters in the stream
	Resultcode = convert.str(); // set 'Result' to the contents of the stream

	string req, num;
	string idcapt;
	req = "SELECT Nom FROM `CAMERA` WHERE ID='";
	req += id;
	req += "'";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int totalrows = mysql_num_rows(result);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("Camera inconnue");
	} else {
		mysql_close(mysql);
		sortErr(171);
	}
	return ("");
}

/**
 * 
 * @param id
 * @return 
 */
string accesBDD::getcamGeo(string nomR) {
	MYSQL *mysql;

	string req, num;
	string nomRcapt;
	req = "SELECT PosGeo FROM `CAMERA` WHERE NomRep='";
	req += nomR;
	req += "'";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
		//if(mysql_real_connect(mysql,"localhost","root","root","Surveillance",0,NULL,0))
	{
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int totalrows = mysql_num_rows(result);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				num += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return num;
		else
			return ("inconnue");
	} else {
		mysql_close(mysql);
		sortErr(173);
	}
	return ("");
}

string accesBDD::getIndexImage() {
	MYSQL *mysql;
	string req, resultat;
	req = "SELECT nomImage FROM IMAGE WHERE idImage = '12'";

	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "appli");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				resultat += row[i];
			}
		}
		mysql_free_result(result);
		mysql_close(mysql);
		return resultat;
	} else {
		mysql_close(mysql);
		sortErr(20);
		return "";
	}
}

string accesBDD::getLastImg(string numImg) {
	MYSQL *mysql;
	string req, resultat, ind;
	if (numImg == "0")
		ind = getIndexImage();
	else {
		ind = numImg;
	}
	req = "SELECT nomImage FROM IMAGE WHERE idImage = ";
	req += ind;

	//fprintf(stderr,"req= %s\n",req.c_str());
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "appli");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		mysql_query(mysql, req.c_str());
		result = mysql_store_result(mysql);
		int numfields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result))) {
			for (int i = 0; i < numfields; i++) {
				resultat += row[i];
			}
		}
		int nbr = result->row_count;
		mysql_free_result(result);
		mysql_close(mysql);
		if (nbr > 0)
			return resultat;
		else
			return " ";
	} else {
		mysql_close(mysql);
		sortErr(21);
		return "";
	}
}

void accesBDD::incIx() {
	MYSQL *mysql;
	//On recupère et on incremente l'index de la derniere image
	// en case 12 de la table IMAGE
	string ind, req;
	ind = getIndexImage();
	int ix = atoi(ind.c_str());
	ix++;
	if (ix == 12)
		ix = 1;
	req = "UPDATE IMAGE SET nomImage = '";
	req += std::to_string(ix);
	req += "' WHERE idImage = 12";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "appli");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		if (mysql_query(mysql, req.c_str()))
			if (Debug)
				fprintf(stderr, "Erreur bdd1: %s\n", mysql_error(mysql));
	} else {
		sortErr(22);
	}
	mysql_close(mysql);
}

void accesBDD::putLastImg(string chImg) {
	MYSQL *mysql;
	string ind, req2;
	//On incremente l'index de la derniere image
	incIx();

	usleep(100000);
	ind = getIndexImage();
	usleep(100000);
	req2 = "UPDATE IMAGE SET nomImage = '";
	req2 += chImg;
	req2 += "', dateIm = NOW() WHERE idImage = ";
	req2 += ind;
	//fprintf(stderr,"%s\nLG=%d\n", req2.c_str(), strlen(req2.c_str()));
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "appli");

	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) {
		if (mysql_query(mysql, req2.c_str()))
			if (Debug)
				fprintf(stderr, "Erreur bdd2: %s\n", mysql_error(mysql));
	} else {
		sortErr(23);
	}
	mysql_close(mysql);
}

void accesBDD::getAdLog(Log* pl) {
	pLog = pl;
}

void accesBDD::initTemperature() {
	MYSQL *mysql;

	string req = "UPDATE `CAPTEUR` SET Valeur = 99.9 WHERE Type = 'Temperature'";
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "");
	if (mysql_real_connect(mysql, host.c_str(), login.c_str(), pwd.c_str(), dbname.c_str(), 0, NULL, 0)) //Si la connexion réussie...
	{
		mysql_query(mysql, req.c_str());
	} else {
		sortErr(24);
	}
	mysql_close(mysql);
}
