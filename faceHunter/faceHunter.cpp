#include <iostream>
#include "opencv2/objdetect.hpp"
#include "opencv2/video.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/face.hpp"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iomanip>
#include <stdlib.h>
#include <boost/python.hpp>
#include <opencv2/opencv.hpp>

using namespace boost::python;
using namespace std;
using namespace cv;
using namespace cv::face;

int genderMascCounter = 0;
int genderFemCounter = 0;
int smileCounter = 0;
int notSmileCounter = 0;

int genderMascPerHour[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
int genderFemPerHour[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
int smilePerHour[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
int notSmilePerHour[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

struct FaceData{
	Rect rCoords;
	int timeInstant;
	int smileFreq;
	int notSmileFreq;
	int mascGendFreq;
	int femGendFreq;
	int gender;
	int expression;
	Mat faceImage;
};

struct LastFace{
	Rect rLastFace;
	Mat face;
	int timeInstant;
};

struct MyRange{
		int index;
		float lower;
		float upper;
};

vector<FaceData> vFaceDataGnd;
vector<FaceData> vFaceDataExp;
vector<LastFace> vLastFaces;

Scalar color = Scalar(0,0,0);

std::vector<Rect> faces;
string face_cascade_name;
string eyes_cascade_name;
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
Mat lvcLogo;

vector<MyRange> vRangesGender, vRangesExp;

HOGDescriptor hog;
struct svm_model *modelExpr;
struct svm_model *modelGend;

Ptr<FaceRecognizer> fisherModelGender;
Ptr<FaceRecognizer> fisherModelExpr;
Ptr<FaceRecognizer> lbphModelGender;
Ptr<FaceRecognizer> lbphModelExpr;
Mat faceExpression;

int globalTimeExpr = 0;
int globalTimeGnd = 0;
const int MAXTIME = 45;
Mat mSor, mNeut;

void clustExpFaces(FaceData fData, float distThrs, int exprClass, int &chosenExprClass);
void cropImageExpr(Mat frameGray, Rect rImg, Mat &img);
void getFaceToGenderRecog(const Mat mFrame, Rect rFace, Mat &mFaceToGR, int windDim);
void clustLastFaces(LastFace fData, float distThrs);
void clustGndFaces(FaceData fData, float distThrs, int exprClass, int &chosenGndClass);
void getGenderImage(Mat &frame, int gndClass, double dist, Rect rFace, int timeInstant, float threeDThrsGnd, float oneDThrsGnd, int 	gndWindSize);
void getExpImage(Mat &frame, int exprClass, double dist, Rect rFace, int timeInstant, float threeDThrsExp, float oneDThrsExp, int expWindSize);
void drawLastFaces(Mat &frame);
Mat detectAndDisplay(Mat frame);

int getQuantLastFaces(){
	return vLastFaces.size();
}
int getTimeInstante(int fIndex){
	return vLastFaces[fIndex].timeInstant;
}

vector<Rect> getFaces(){
	return faces;
}

int getMascGendFreq(int indice){return vFaceDataGnd[indice].mascGendFreq;}
int getSmileFreq(int indice){return vFaceDataGnd[indice].smileFreq;}
int getFemGendFreq(int indice){return vFaceDataGnd[indice].femGendFreq;}
int getNotSmileFreq(int indice){return vFaceDataGnd[indice].notSmileFreq;}

int getMascGendFreqvFaceDataExp(int indice){return vFaceDataExp[indice].mascGendFreq;}
int getSmileFreqvFaceDataExp(int indice){return vFaceDataExp[indice].smileFreq;}
int getFemGendFreqvFaceDataExp(int indice){return vFaceDataExp[indice].femGendFreq;}
int getNotSmileFreqvFaceDataExp(int indice){return vFaceDataExp[indice].notSmileFreq;}


void getFaceToGenderRecog(const Mat mFrame, Rect rFace, Mat &mFaceToGR, int windDim){
	int ten = rFace.width / 10;
	int halfTen = ten / 2;
	int x = rFace.x - halfTen;
	int y = rFace.y - halfTen;
	int width = rFace.width + ten;
	int height = rFace.height + ten;

	if (x < 0) x = 0;
	if (y < 0) y = 0;

	if ((width + x) > mFrame.cols) width = mFrame.cols - x - 1;
	if ((height + y) > mFrame.rows) height = mFrame.rows - y - 1;

	Mat faceROI = Mat(mFrame, Rect(x, y, width, height));
	resize(faceROI, mFaceToGR, Size(windDim, windDim), 0, 0, INTER_CUBIC);
}

void cropImageExpr(Mat frameGray, Rect rImg, Mat &img) {
	double x = rImg.x;
	double y = rImg.y;
	double w = rImg.width;
	double h = rImg.height;

	rImg.x = x + 0.1*w;
	rImg.y = y + 0.1*h;
	rImg.width = 0.8*w;
	rImg.height = 0.8*h;

	frameGray(rImg).copyTo(img);
}

void clustExpFaces(FaceData fData, float distThrs, int exprClass, int &chosenExprClass) {
	chosenExprClass = 0;

	for(int i = 0; i < vFaceDataExp.size(); i++) {
		if(vFaceDataExp[i].timeInstant > MAXTIME) {
		  vFaceDataExp.erase(vFaceDataExp.begin()+i);
		}
	}

	double minDist = 61;
	int minIndex = -1;

	for(int i = 0; i < vFaceDataExp.size(); i++){
		double dx = pow(fData.rCoords.x - vFaceDataExp[i].rCoords.x, 2.0);
		double dy = pow(fData.rCoords.y - vFaceDataExp[i].rCoords.y, 2.0);
		double dt = pow(fData.timeInstant - vFaceDataExp[i].timeInstant, 2.0);
		double dist = sqrt(dx + dy + dt);

		if(dist < minDist){
			minDist = dist;
			minIndex = i;
		}
	}

	if(minDist < distThrs && minIndex != -1){
		vFaceDataExp[minIndex].rCoords.x = fData.rCoords.x;
		vFaceDataExp[minIndex].rCoords.y = fData.rCoords.y;
		vFaceDataExp[minIndex].timeInstant = fData.timeInstant;

		if(exprClass == 0){
			vFaceDataExp[minIndex].smileFreq = vFaceDataExp[minIndex].smileFreq + 1;
		}
		else{
			vFaceDataExp[minIndex].notSmileFreq = vFaceDataExp[minIndex].notSmileFreq + 1;
		}

		if(vFaceDataExp[minIndex].smileFreq > vFaceDataExp[minIndex].notSmileFreq){
			chosenExprClass = 0;
		}
		else{
			chosenExprClass = 1;
		}

	}
	else{
		if(exprClass == 0){
			chosenExprClass = 0;
			fData.smileFreq = 1;
			fData.notSmileFreq = 0;
			smileCounter++;
		}
		else{
			chosenExprClass = 1;
			fData.smileFreq = 0;
			fData.notSmileFreq = 1;
			notSmileCounter++;
		}

		vFaceDataExp.push_back(fData);
	}
}

void clustLastFaces(LastFace fData, float distThrs){
	double minDist = 61;
	int minIndex = -1;

	for(int i = 0; i < vLastFaces.size(); i++){
		double dx = pow(fData.rLastFace.x - vLastFaces[i].rLastFace.x, 2.0);
		double dy = pow(fData.rLastFace.y - vLastFaces[i].rLastFace.y, 2.0);
		double dt = pow(fData.timeInstant - vLastFaces[i].timeInstant, 2.0);
		double dist = sqrt(dx + dy + dt);

		if(dist < minDist){
			minDist = dist;
			minIndex = i;
		}
	}

	if(minDist < distThrs && minIndex != -1){
		vLastFaces[minIndex].rLastFace.x = fData.rLastFace.x;
		vLastFaces[minIndex].rLastFace.y = fData.rLastFace.y;
		vLastFaces[minIndex].timeInstant = fData.timeInstant;
	}
	else{
		if(vLastFaces.size() < 8){
			vLastFaces.push_back(fData);
		}
		else{
			vLastFaces.erase(vLastFaces.begin());
			vLastFaces.push_back(fData);
		}
	}
}

void clustGndFaces(FaceData fData, float distThrs, int gndClass, int &chosenGndClass){
	chosenGndClass = 0;
	double minDist = 61;
	int minIndex = -1;

	for(int i = 0; i < vFaceDataGnd.size(); i++){
		double dx = pow(fData.rCoords.x - vFaceDataGnd[i].rCoords.x, 2.0);
		double dy = pow(fData.rCoords.y - vFaceDataGnd[i].rCoords.y, 2.0);
		double dt = pow(fData.timeInstant - vFaceDataGnd[i].timeInstant, 2.0);
		double dist = sqrt(dx + dy + dt);

		if(dist < minDist){
			minDist = dist;
			minIndex = i;
		}
	}
	if(minDist < distThrs && minIndex != -1){
		vFaceDataGnd[minIndex].rCoords.x = fData.rCoords.x;
		vFaceDataGnd[minIndex].rCoords.y = fData.rCoords.y;
		vFaceDataGnd[minIndex].timeInstant = fData.timeInstant;
		
		if(gndClass == 0){
			vFaceDataGnd[minIndex].smileFreq = vFaceDataGnd[minIndex].smileFreq + 1;
		}
		else{
			vFaceDataGnd[minIndex].notSmileFreq = vFaceDataGnd[minIndex].notSmileFreq + 1;
		}

		if(vFaceDataGnd[minIndex].smileFreq > vFaceDataGnd[minIndex].notSmileFreq){
			chosenGndClass = 0;
		}
		else{
			chosenGndClass = 1;
		}

		if(fData.gender == 0){
			vFaceDataGnd[minIndex].femGendFreq = vFaceDataGnd[minIndex].femGendFreq + 1;
		}
		else{
			if(fData.gender == 1){
				vFaceDataGnd[minIndex].mascGendFreq = vFaceDataGnd[minIndex].mascGendFreq + 1;
			}
		}
	}
	else{
		if(gndClass == 0){
			chosenGndClass = 0;
			fData.femGendFreq = 1;
			fData.mascGendFreq = 0;
			genderFemCounter++;
		}
		else{
			chosenGndClass = 1;
			fData.femGendFreq = 0;
			fData.mascGendFreq = 1;
			genderMascCounter++;
		}
		vFaceDataGnd.push_back(fData);
	}
}

void getGenderImage(Mat &frame, int gndClass, double dist, Rect rFace, int timeInstant, float threeDThrsGnd, float oneDThrsGnd, int gndWindSize){
	FaceData fData;
	fData.rCoords = rFace;
	fData.timeInstant = timeInstant;
	fData.smileFreq = 0;
	fData.notSmileFreq = 0;
	fData.gender = gndClass;

	int chosenGndClass = -1;
	clustGndFaces(fData, threeDThrsGnd, gndClass, chosenGndClass);

	if(chosenGndClass == 1 && dist < oneDThrsGnd){
		color = Scalar(255,0,0);
	    string str("M");

	    if(rFace.x - 10 >= 0 && rFace.y - 10 >=0){
		    cv::putText(frame, str, cv::Point(rFace.x - 10, rFace.y - 10), CV_FONT_HERSHEY_PLAIN, 2, color,5);
		}
	}
	else{
		if(chosenGndClass == 0  && dist < oneDThrsGnd){
		    color = Scalar(0,0,255);
		    string str("F");
		    
			if(rFace.x - 10 >= 0 && rFace.y - 10 >=0){
		    	cv::putText(frame, str, cv::Point(rFace.x - 10, rFace.y - 10), CV_FONT_HERSHEY_PLAIN, 2, color,5);
			}
		}
		else{
		    string str("I");
		    color = Scalar(0,0,0);
		
		    if(rFace.x - 10 >= 0 && rFace.y - 10 >=0){
		    	cv::putText(frame, str, cv::Point(rFace.x - 10, rFace.y - 10), CV_FONT_HERSHEY_PLAIN, 2, color,5);
			}
		}
	}
}

void getExpImage(Mat &frame, int exprClass, double dist, Rect rFace, int timeInstant, float threeDThrsExp, float oneDThrsExp, int expWindSize){
	FaceData fData;
	fData.rCoords = rFace;
	fData.timeInstant = timeInstant;
	fData.smileFreq = 0;
	fData.notSmileFreq = 0;

	int chosenExprClass = -1;
	clustExpFaces(fData, threeDThrsExp, exprClass, chosenExprClass);

	if(chosenExprClass == 0 && dist < oneDThrsExp){
		Rect rect;
		rect.x = rFace.x + rFace.width - expWindSize;
		rect.y = rFace.y - expWindSize;
		rect.width = mSor.cols;
		rect.height = mSor.rows;
		mSor.copyTo(faceExpression);

		if(rect.x >= 0 && rect.y >= 0 && rect.x + rect.width < frame.cols && rect.y + rect.width <  frame.rows){
			faceExpression.copyTo(frame(rect));
		}
	}
	else{
		if(chosenExprClass == 1  && dist < oneDThrsExp){
			Rect rect;
			rect.x = rFace.x + rFace.width - expWindSize;
			rect.y = rFace.y - expWindSize;
			rect.width = mNeut.cols;
			rect.height = mNeut.rows;
			mNeut.copyTo(faceExpression);

			if(rect.x >= 0 && rect.y >= 0 && rect.x + rect.width < frame.cols && rect.y + rect.width <  frame.rows){
				faceExpression.copyTo(frame(rect));
			}
		}
	}
}

void drawLastFaces(Mat &frame){
	assert(vLastFaces.size() <= 8);

	for(int i = 0; i < vLastFaces.size(); i++){
		Mat aux;
		resize(vLastFaces[i].face, aux, Size(60,60));
		aux.copyTo(frame(Rect(frame.cols-60, i*60,60,60)));
	}

	rectangle( frame, Point( frame.cols-60, 0 ), Point( frame.cols, frame.rows), Scalar(128,0,128), 2, 16 );
}

Mat detectAndDisplay(Mat frame){
	mNeut.copyTo(faceExpression);
	string legenda = string("M: Masculino");

	cv::putText(frame, legenda, cv::Point( 10, 30), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255,0,0),2);
	legenda = string("F: Feminino");
	
	cv::putText(frame, legenda, cv::Point( 10, 50), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0,0,255),2);
	legenda = string("I: Indeterminado");
	
	cv::putText(frame, legenda, cv::Point( 10, 70), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0,0,0),2);
	lvcLogo.copyTo(frame(Rect(05, frame.rows-75,75,75)));

	int genderWindSize = 50;
	int expWindSize = 50;
	int nBins = 4;
	int P = 4;
	int R = 1;

	Mat frame_gray;
	cvtColor( frame, frame_gray, COLOR_BGR2GRAY );

	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 5, 0|CASCADE_SCALE_IMAGE, Size(1.2*genderWindSize, 1.2*genderWindSize) );

	for(size_t i = 0; i < faces.size(); i++ ){
		LastFace fData;
		fData.rLastFace = faces[i];
		fData.timeInstant = globalTimeGnd;

		float distClustLast = 15;

		Mat faceAux;
		frame(faces[i]).copyTo(faceAux);
		fData.face = faceAux;

		clustLastFaces(fData, distClustLast);

		Mat imgResizedGender, imgResized;
		Mat auxM;

		getFaceToGenderRecog(frame_gray, faces[i], imgResizedGender, genderWindSize);
		cropImageExpr(frame_gray, faces[i], auxM);
		resize(auxM, imgResized, Size(expWindSize,expWindSize));

		float thrsExp = 30;
		float thrsGend = 15;

		float threeDThrsExp = 50;
		float threeDThrsGnd = 50;

		double predicted_confidence_exp = 0;
		int exprClass = -1;

		equalizeHist(imgResized, imgResized);
		lbphModelExpr->predict(imgResized, exprClass, predicted_confidence_exp);

		getExpImage(frame, exprClass, predicted_confidence_exp, faces[i], globalTimeExpr, threeDThrsExp, thrsExp, expWindSize);
		globalTimeExpr++;

		double predicted_confidence_gender = 0;
		int gender = -1;

		equalizeHist(imgResizedGender, imgResizedGender);
		lbphModelGender->predict(imgResizedGender, gender, predicted_confidence_gender);

		getGenderImage(frame, gender, predicted_confidence_gender, faces[i], globalTimeGnd, threeDThrsGnd, thrsGend, genderWindSize);
		rectangle( frame, Point( faces[i].x, faces[i].y ), Point( faces[i].x + faces[i].width, faces[i].y + faces[i].height), color, 2, 16 );
	}


	drawLastFaces(frame);

	if(globalTimeExpr > MAXTIME){
		globalTimeExpr = 0;
	}

	if(globalTimeGnd > MAXTIME){
		globalTimeGnd = 0;
	}
	else{
		globalTimeGnd++;
	}

	return frame;
}

void loadDependencies(){
	face_cascade_name = "faceHunter/data/lbpcascades/lbpcascade_frontalface.xml";
	eyes_cascade_name = "faceHunter/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";

	fisherModelGender = createEigenFaceRecognizer();
	fisherModelExpr = createEigenFaceRecognizer();

	lbphModelExpr = createLBPHFaceRecognizer();
	lbphModelExpr->load("faceHunter/models/expModel50x50p8r1h4v4.yml");

	lbphModelGender = createLBPHFaceRecognizer();
	lbphModelGender->load("faceHunter/models/gndModel50x50p8r1h4v4.yml");

	if(!face_cascade.load(face_cascade_name)){
		printf("--(!)Error loading face cascade\n");
		exit(0);
	}
	if(!eyes_cascade.load(eyes_cascade_name)){
		printf("--(!)Error loading eyes cascade\n");
		exit(0);
	}

	mSor = imread("images/smile.png");
	mNeut = imread("images/not_smile.png");
	lvcLogo = imread("images/lpc_logo.jpg");

	if(mSor.empty() || mNeut.empty()){
		cout<<"Não carregou os emoticons"<<endl;
		exit(0);
	}

	resize(mSor, mSor, Size(50,50));
	resize(mNeut, mNeut, Size(50,50));
	resize(lvcLogo, lvcLogo, Size(75,75));
}

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c){
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len){
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while(in_len--){
		char_array_3[i++] = *(bytes_to_encode++);
		
		if(i == 3){
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(j = 0; (j <4) ; j++){
				ret += base64_chars[char_array_4[j]];
			}

			i = 0;
		}
	}

	if(i){
		for(j = i; j < 3; j++){
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++){
			ret += base64_chars[char_array_4[j]];
		}

		while((i++ < 3)){
			ret += '=';
		}
	}

	return ret;
}

std::string base64_decode(std::string const& encoded_string){
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while(in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])){
		char_array_4[i++] = encoded_string[in_], in_++;
		
		if(i ==4){
			for(j = 0; j <4; j++){
				char_array_4[j] = base64_chars.find(char_array_4[j]);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for(j = 0; (j < 3); j++){
				ret += char_array_3[j];
			}
			i = 0;
		}
	}

	if(i){
		for (j = i; j <4; j++){
			char_array_4[j] = 0;
		}

		for (j = 0; j <4; j++){
			char_array_4[j] = base64_chars.find(char_array_4[j]);
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++){
			ret += char_array_3[j];
		}
	}

	return ret;
}

string faceHunterAction(string encoded){
	string dec_jpg =  base64_decode(encoded);
    std::vector<uchar> data(dec_jpg.begin(), dec_jpg.end());
    cv::Mat img = cv::imdecode(cv::Mat(data), 1);

	img = detectAndDisplay(img);

	std::vector<uchar> buf;
	cv::imencode(".jpg", img, buf);
	auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
	encoded = base64_encode(enc_msg, buf.size());

	return encoded;
}

int getGenderMascCounter(){
	return genderMascCounter;
}

int getGenderFemCounter(){
	return genderFemCounter;
}

int getSmileCounter(){
	return smileCounter;
}

int getNotSmileCounter(){
	return notSmileCounter;
}

BOOST_PYTHON_MODULE(faceHunter){
   	def("faceHunterAction", faceHunterAction);
	def("loadDependencies", loadDependencies);
	def("getMascGendFreq", getMascGendFreq);
	def("getSmileFreq", getSmileFreq);
	def("getGenderMascCounter", getGenderMascCounter);
	def("getGenderFemCounter", getGenderFemCounter);
	def("getSmileCounter", getSmileCounter);
	def("getNotSmileCounter", getNotSmileCounter);
}
