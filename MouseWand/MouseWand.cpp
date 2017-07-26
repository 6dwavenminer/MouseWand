/*
MouseWand.ccp
Version:1.2.4
Created by Oliver Thurgood on: 5/10/2015
Last edited by Oliver Thurgood on: 31/10/2015

Using the 3/3/3 phidget IMU, program calculates velocity parrallel and perpendicular to gravity.
Uses these values to control computers mouse cursor.
Velocity parrallel to gravity controls Y position of mouse cursor.
Velocity perpendicular to gravity contols X position of mouse cursor.
*/

#include <iostream>
#include <Windows.h>
#include <time.h>
#include <phidget21.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <math.h>
//#include <cstdio>

#define Pi 3.14159265358979323846

HWND hWindow;

/*
Screen_GetScaleFactor(double &dScaleFactorX, double &dScaleFactorY)
Calculates the X and Y scale factors for the current computer screen.
*/
int Screen_GetScaleFactor(double &dScaleFactorX, double &dScaleFactorY) {
	int iScreenResX = GetSystemMetrics(SM_CXSCREEN);
	int iScreenResY = GetSystemMetrics(SM_CYSCREEN);
	dScaleFactorX = 65536 / (double)(iScreenResX - 1);
	dScaleFactorY = 65536 / (double)(iScreenResY - 1);
	//printf("XScale:%G\n", dScaleFactorX);
	//printf("YScale:%G\n", dScaleFactorY);
	return 0;
}

/*
Mouse_GetPos(int &CursorPosX, int &CursorPosY)
Reads the current computer mouse positon then returns it as a X and Y postions.
*/
int Mouse_GetPos(int &CursorPosX, int &CursorPosY) {
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	CursorPosX = CursorPos.x;
	CursorPosY = CursorPos.y;
	//std::system("cls");
	//printf("X:%d\n", CursorPosX);
	//printf("Y:%d\n", CursorPosY);
	return 0;
}

/*
Mouse_SetPos(int CursorPosX,int CursorPosY)
Sets the positon of the mouse cursor.
*/
int Mouse_SetPos(int CursorPosX, int CursorPosY) {
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dx = (LONG)CursorPosX;
	Input.mi.dy = (LONG)CursorPosY;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	SendInput(1, &Input, sizeof(INPUT));
	return 0;
}

/*
Mouse_LeftClick(void)
Left clicks the mouse
*/
int Mouse_LeftClick(void) {
	INPUT Input = { 0 };
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
	SendInput(1, &Input, sizeof(INPUT));
	return 0;
}

/*
Mouse_RightClick(void)
Right clicks the mouse
*/
int Mouse_RightClick(void) {
	INPUT Input = { 0 };
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
	SendInput(1, &Input, sizeof(INPUT));
	return 0;
}

/*
DetectClick(double dScaleFactorX, double dScaleFactorY)
Monitors the cursor position, if it moves faster than sensitivity value the cursor positon is reset and the mouse is left clicked.
*/
int DetectClick(double dScaleFactorX, double dScaleFactorY) {
	int StartingCursorPosX;
	int StartingCursorPosY;
	int CursorPosX;
	int CursorPosY;
	int Counter = 0;
	int CursorPosXMin = 5000;
	int CursorPosXMax = 0;
	int CursorPosYMin = 5000;
	int CursorPosYMax = 0;
	int Sensitivity = 7;											//Max-Min left click sensitivity
	Mouse_GetPos(StartingCursorPosX, StartingCursorPosY);		//Gets cursor starting position
																//Gets a number of positon samples and stores the maximum and minimum positions
	while (Counter <= 2) {
		Mouse_GetPos(CursorPosX, CursorPosY);
		if (CursorPosX < CursorPosXMin) {
			CursorPosXMin = CursorPosX;
		}
		if (CursorPosX > CursorPosXMax) {
			CursorPosXMax = CursorPosX;
		}
		if (CursorPosY < CursorPosYMin) {
			CursorPosYMin = CursorPosY;
		}
		if (CursorPosY > CursorPosYMax) {
			CursorPosYMax = CursorPosY;
		}
		Counter++;
	}
	//std::system("cls");
	//printf("Min:%d\n", CursorPosXMin);
	//printf("Max:%d\n", CursorPosXMax);
	//If the difference between the maximum and minimum is above the sensitivity, the mouse is left clicked.
	if (((CursorPosXMin + Sensitivity)<CursorPosXMax) || ((CursorPosYMin + Sensitivity)<CursorPosYMax)) {
		printf("CLICK\n");
		Sleep(200);
		Mouse_SetPos(StartingCursorPosX * (int)dScaleFactorX, StartingCursorPosY * (int)dScaleFactorY);
		Sleep(1);
		Mouse_LeftClick();
		Sleep(200);
	}
	return 0;
}
/*
GetAcceleration(CPhidgetSpatialHandle Spatial,double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ)
Gets acceleration data from the 3/3/3 IMU.
*/
int GetAcceleration(CPhidgetSpatialHandle Spatial, double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ) {
	CPhidgetSpatial_getAcceleration(Spatial, 0, &dAccelerationX);
	CPhidgetSpatial_getAcceleration(Spatial, 1, &dAccelerationY);
	CPhidgetSpatial_getAcceleration(Spatial, 2, &dAccelerationZ);
	return 0;
}

/*
GetdResultantAcceleration(double dAccelerationX, double dAccelerationY, double dAccelerationZ)
Uses the acceleration data to calculate the resultant acceleration.
*/
double GetdResultantAcceleration(double dAccelerationX, double dAccelerationY, double dAccelerationZ) {
	double dResultantAcceleration = sqrt((dAccelerationX*dAccelerationX) + (dAccelerationY*dAccelerationY) + (dAccelerationZ*dAccelerationZ));
	return dResultantAcceleration;
}

/*
GetAngularPos(double &dAngleX,double &dAngleY,double &dAngleZ,double dAccelerationX,double dAccelerationY, double dAccelerationZ)
Uses the acceleration data to calculate the IMU's current angle relative to the acceleration caused by the force of gravity.
*/
int GetAngularPos(double &dAngleX, double &dAngleY, double &dAngleZ, double dAccelerationX, double dAccelerationY, double dAccelerationZ) {
	double dAngleXBuffer = dAngleX;
	double dAngleYBuffer = dAngleY;
	double dAngleZBuffer = dAngleZ;

	//Calculates relative X angle
	if (((dAngleY > 85)&(dAngleY < 95))&((dAngleZ<-85)&(dAngleZ>-95)) | ((dAngleY<-85)&(dAngleY>-95))&((dAngleZ>85)&(dAngleZ < 95))) {
		//If X angle is parrallel to the force of gravity, when angle cannot be accuratly calculated, skip calculation (gyro used instead).
	}
	else {	//Not
		dAngleXBuffer = (atan2(dAccelerationY, dAccelerationZ) / Pi) * 180;
		if ((dAngleXBuffer >= -180)&(dAngleXBuffer <= 180)) {
			dAngleX = dAngleXBuffer;
		}
	}

	//Calculates relative Y angle
	if (((dAngleX > 85)&(dAngleX < 95))&((dAngleZ<5)&(dAngleZ>-5)) | ((dAngleX<-85)&(dAngleX>-95))&((dAngleZ>175)&(dAngleZ < -175))) {
		//If Y angle is parrallel to the force of gravity, when angle cannot be accuratly calculated, skip calculation (gyro used instead).
	}
	else {	//Not
		dAngleYBuffer = (atan2(dAccelerationX, dAccelerationZ) / Pi) * 180;
		if ((dAngleYBuffer >= -180)&(dAngleYBuffer <= 180)) {
			dAngleY = -dAngleYBuffer;
		}
	}

	//Calculates relative Z angle
	if (((dAngleY > -5)&(dAngleY < 5))&((dAngleZ<5)&(dAngleZ>-5)) | ((dAngleY < -175)&(dAngleY>175))&((dAngleZ>175)&(dAngleZ < -175))) {
		//If Z angle is parrallel to the force of gravity, when angle cannot be accuratly calculated, skip calculation (gyro used instead).
	}
	else {	//Not
		dAngleZBuffer = (atan2(dAccelerationX, dAccelerationY) / Pi) * 180;
		if ((dAngleZBuffer >= -180)&(dAngleZBuffer <= 180)) {
			dAngleZ = dAngleZBuffer;
		}
	}
	return 0;
}

/*
GetAngularRate(CPhidgetSpatialHandle Spatial,double &dAngularRateX,double &dAngularRateY,double &dAngularRateZ)
Gets the angular rate of the IMU from the three gyroscopes
*/
int GetAngularRate(CPhidgetSpatialHandle Spatial, double &dAngularRateX, double &dAngularRateY, double &dAngularRateZ) {
	CPhidgetSpatial_getAngularRate(Spatial, 0, &dAngularRateX);
	CPhidgetSpatial_getAngularRate(Spatial, 1, &dAngularRateY);
	CPhidgetSpatial_getAngularRate(Spatial, 2, &dAngularRateZ);
	return 0;
}

/*
AccelerationGravityCorrection(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ)
Attempts to negate the acceleration due to gravity
*/
int AccelerationGravityCorrection(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ) {
	//Negates X acceleration due to gravity
	if ((dAccelerationX >= 0.01) | (dAccelerationX <= -0.01)) {
		if (((dAngleZ < 180)&(dAngleZ>0)) | ((dAngleY < 0)&(dAngleY>-180))) {
			dAccelerationX += (0.981 * cos(((dAngleZ + 90)*Pi) / 180)*cos(((dAngleY + 90)*Pi) / 180));
		}
		else {
			dAccelerationX -= (1.020 * cos(((dAngleZ + 90)*Pi) / 180)*cos(((dAngleY + 90)*Pi) / 180));
		}
	}
	else {
		dAccelerationX = 0;
	}

	//Negates Y acceleration due to gravity
	if ((dAccelerationY >= 0.01) | (dAccelerationY <= -0.01)) {
		if (((dAngleZ < 90)&(dAngleZ>-90)) | ((dAngleX < 180)&(dAngleX>0))) {
			dAccelerationY -= (1.014 * cos(((dAngleX - 90)*Pi) / 180)*cos((dAngleZ*Pi) / 180));
		}
		else {
			dAccelerationY += (0.988 * cos(((dAngleX - 90)*Pi) / 180)*cos((dAngleZ*Pi) / 180));
		}
	}
	else {
		dAccelerationY = 0;
	}

	//Negates Z acceleration due to gravity
	if ((dAccelerationZ >= 0.01) || (dAccelerationZ <= -0.01)) {

		if (((dAngleX < 90)&(dAngleX>-90)) | ((dAngleY < 90)&(dAngleY>-90))) {
			dAccelerationZ -= (1.005*cos((dAngleX*Pi) / 180)*cos((dAngleY*Pi) / 180));
		}
		else {
			dAccelerationZ += (0.995*cos((dAngleX*Pi) / 180)*cos((dAngleY*Pi) / 180));
		}
	}
	else {
		dAccelerationZ = 0;
	}
	return 0;
}

/*
AccelerationCorrection(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ,double &dGlobalAccelerationZ)
Calculates the acceleration parrallel to force of gravity
*/
int AccelerationCorrection(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ, double &dGlobalAccelerationZ) {
	dGlobalAccelerationZ = 0;
	//Calculates the acceleration parrallel to force of gravity using the X acceleration
	if (((dAngleZ < 180)&(dAngleZ>0)) | ((dAngleY < 0)&(dAngleY>-180))) {
		dGlobalAccelerationZ += (dAccelerationX * cos(((dAngleZ + 90)*Pi) / 180)*cos(((dAngleY + 90)*Pi) / 180));
	}
	else {
		dGlobalAccelerationZ -= (dAccelerationX * cos(((dAngleZ + 90)*Pi) / 180)*cos(((dAngleY + 90)*Pi) / 180));
	}

	//Calculates the acceleration parrallel to force of gravity using the Y acceleration
	if (((dAngleZ < 90)&(dAngleZ>-90)) | ((dAngleX < 180)&(dAngleX>0))) {
		dGlobalAccelerationZ -= (dAccelerationY * cos(((dAngleX - 90)*Pi) / 180)*cos((dAngleZ*Pi) / 180));
	}
	else {
		dGlobalAccelerationZ += (dAccelerationY * cos(((dAngleX - 90)*Pi) / 180)*cos((dAngleZ*Pi) / 180));
	}

	//Calculates the acceleration parrallel to force of gravity using the Z acceleration
	if (((dAngleX < 90)&(dAngleX>-90)) | ((dAngleY < 90)&(dAngleY>-90))) {
		dGlobalAccelerationZ -= (dAccelerationZ * cos((dAngleX*Pi) / 180)*cos((dAngleY*Pi) / 180));
	}
	else {
		dGlobalAccelerationZ += (dAccelerationZ * cos((dAngleX*Pi) / 180)*cos((dAngleY*Pi) / 180));
	}
	return 0;
}
/*
GetMagneticField(CPhidgetSpatialHandle Spatial, double &dMagneticFieldX, double &dMagneticFieldY, double &dMagneticFieldZ)
Gets the magnetometric values
*/
int GetMagneticField(CPhidgetSpatialHandle Spatial, double &dMagneticFieldX, double &dMagneticFieldY, double &dMagneticFieldZ) {
	CPhidgetSpatial_getMagneticField(Spatial, 0, &dMagneticFieldX);
	CPhidgetSpatial_getMagneticField(Spatial, 1, &dMagneticFieldY);
	CPhidgetSpatial_getMagneticField(Spatial, 2, &dMagneticFieldZ);
	//printf("MagX:%G\n", dMagneticFieldX);
	//printf("MagY:%G\n", dMagneticFieldY);
	//printf("MagZ:%G\n", dMagneticFieldZ);
	return 0;
}

/*
GetAngularPosMagnetic(double &dAngleX, double &dAngleY, double &dAngleZ, double dMagneticFieldX, double dMagneticFieldY, double dMagneticFieldZ)
Calculates the IMU's angle relative to the Earths magnetic field
*/
int GetAngularPosMagnetic(double &dAngleX, double &dAngleY, double &dAngleZ, double dMagneticFieldX, double dMagneticFieldY, double dMagneticFieldZ) {
	double dAngleXBuffer = dAngleX;
	double dAngleYBuffer = dAngleY;
	double dAngleZBuffer = dAngleZ;

	dAngleXBuffer = (atan2(dMagneticFieldY, dMagneticFieldZ) / Pi) * 180;
	if ((dAngleXBuffer >= -180)&(dAngleXBuffer <= 180)) {
		dAngleX = dAngleXBuffer;
	}

	dAngleYBuffer = (atan2(dMagneticFieldX, dMagneticFieldZ) / Pi) * 180;
	if ((dAngleYBuffer >= -180)&(dAngleYBuffer <= 180)) {
		dAngleY = -dAngleYBuffer;
	}

	dAngleZBuffer = (atan2(dMagneticFieldX, dMagneticFieldY) / Pi) * 180;
	if ((dAngleZBuffer >= -180)&(dAngleZBuffer <= 180)) {
		dAngleZ = dAngleZBuffer;
	}
	return 0;
}

/*
AccelerationCorrectionMag(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dMagAngleX, double dMagAngleY, double dMagAngleZ, double &dGlobalAccelerationNS)
Calculates the IMU's acceleration relative to the Earths magnetic field
*/
int AccelerationCorrectionMag(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dMagAngleX, double dMagAngleY, double dMagAngleZ, double &dGlobalAccelerationNS) {
	dGlobalAccelerationNS = 0;
	if ((dMagAngleZ < 90)&(dMagAngleZ>-90)) {
		dGlobalAccelerationNS += (dAccelerationY * cos((dMagAngleZ*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationY * cos((dMagAngleZ*Pi) / 180));
	}
	if ((dMagAngleZ < 180)&(dMagAngleZ>0)) {
		dGlobalAccelerationNS += (dAccelerationY*sin((dMagAngleZ*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationY*sin((dMagAngleZ*Pi) / 180));
	}
	//

	if ((dMagAngleY < 90)&(dMagAngleY>-90)) {
		dGlobalAccelerationNS += (dAccelerationY * cos((dMagAngleZ*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationY * cos((dMagAngleZ*Pi) / 180));
	}
	if ((dMagAngleY < 180)&(dMagAngleY>0)) {
		dGlobalAccelerationNS += (dAccelerationX*sin((dMagAngleY*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationX*sin((dMagAngleY*Pi) / 180));
	}
	//

	if ((dMagAngleX < 90)&(dMagAngleX>-90)) {
		dGlobalAccelerationNS += (dAccelerationZ * cos((dMagAngleX*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationZ * cos((dMagAngleX*Pi) / 180));
	}
	if ((dMagAngleX < 180)&(dMagAngleX>0)) {
		dGlobalAccelerationNS += (dAccelerationY*sin((dMagAngleX*Pi) / 180));
	}
	else {
		dGlobalAccelerationNS += (-dAccelerationY*sin((dMagAngleX*Pi) / 180));
	}

	return 0;
}

/*
AccelerationCorrectionX(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ, double &dGlobalAccelerationX)
Calculates the acceleration perpendicular to force of gravity
*/
int AccelerationCorrectionX(double &dAccelerationX, double &dAccelerationY, double &dAccelerationZ, double dAngleX, double dAngleY, double dAngleZ, double &dGlobalAccelerationX) {
	dGlobalAccelerationX = 0;
	//Calculates the acceleration perpendicular to force of gravity using the X acceleration
	if (((dAngleZ < 180)&(dAngleZ>0)) | ((dAngleY < 0)&(dAngleY>-180))) {
		dGlobalAccelerationX += (dAccelerationX * cos(((dAngleZ)*Pi) / 180)*cos(((dAngleY)*Pi) / 180));
	}
	else {
		dGlobalAccelerationX -= (dAccelerationX * cos(((dAngleZ)*Pi) / 180)*cos(((dAngleY)*Pi) / 180));
	}
	//Calculates the acceleration perpendicular to force of gravity using the Y acceleration
	if (((dAngleZ < 90)&(dAngleZ>-90)) | ((dAngleX < 180)&(dAngleX>0))) {
		dGlobalAccelerationX -= (dAccelerationY * cos(((dAngleX - 180)*Pi) / 180)*cos(((dAngleZ - 90)*Pi) / 180));
	}
	else {
		dGlobalAccelerationX += (dAccelerationY * cos(((dAngleX - 180)*Pi) / 180)*cos(((dAngleZ - 90)*Pi) / 180));
	}
	//Calculates the acceleration perpendicular to force of gravity using the Z acceleration
	if (((dAngleX < 90)&(dAngleX>-90)) | ((dAngleY < 90)&(dAngleY>-90))) {
		dGlobalAccelerationX -= (dAccelerationZ * cos(((dAngleX - 90)*Pi) / 180)*cos(((dAngleY - 90)*Pi) / 180));
	}
	else {
		dGlobalAccelerationX += (dAccelerationZ * cos(((dAngleX - 90)*Pi) / 180)*cos(((dAngleY - 90)*Pi) / 180));
	}
	return 0;
}

int main()
{
	ShowCursor(1);
	POINT cursorPos = { 0 };
	INPUT Input = { 0 };

	//Declare an spatial handle
	CPhidgetSpatialHandle Spatial = 0;

	//create the spatial object
	CPhidgetSpatial_create(&Spatial);

	CPhidget_open((CPhidgetHandle)Spatial, -1);

	double dAngularRateX;
	double dAngleX = 0;
	double dAngularRateY;
	double dAngleY = 0;
	double dAngularRateZ;
	double dAngleZ = 0;

	//double dMagneticFieldX=0;
	//double dMagneticFieldY=0;
	//double dMagneticFieldZ=0;
	//double dMagAngleX = 0;
	//double dMagAngleY = 0;
	//double dMagAngleZ = 0;
	double dGlobalAccelerationNS = 0;
	double dGlobalVelocityNS = 0;

	double WandPosChangeX = 0;
	double WandPosChangeY = 0;

	double dGlobalVelocityZ = 0;
	double dGlobalVelocityZBuffer = 0;
	//double dGlobalVelocityNSBuffer = 0;
	double dGlobalVelocityX = 0;
	double dGlobalVelocityXBuffer = 0;
	double dGlobalAccelerationZ = 0;
	double dGlobalAccelerationX;
	double dRunTime = 0;
	int iZeroVelocityCounter = 0;
	//int iZeroVelocityCounterNS = 0;
	int iZeroVelocityCounterX = 0;

	//Setup for gnuplot
	FILE *gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -persistent", "w");
	if (gnuplotPipe != NULL) {
		fputs("set terminal win\n", gnuplotPipe);
		fputs("plot '-' u 1:2\n", gnuplotPipe);
	}
	else {
		printf("ERROR, Could not open gnuplot.exe");
	}

	//Main loop
	while (1) {
		clock_t start = clock();
		std::system("cls");			//Clears the terminal

		GetAngularRate(Spatial, dAngularRateX, dAngularRateY, dAngularRateZ);
		dAngleX += (dAngularRateX/45);
		dAngleY += (dAngularRateY/45);
		dAngleZ += (dAngularRateZ/45);
		printf("X Angular Rate: %G\n", dAngularRateX);
		printf("Y Angular Rate: %G\n", dAngularRateY);
		printf("Z Angular Rate: %G\n", dAngularRateZ);

		double dAccelerationX;
		double dAccelerationY;
		double dAccelerationZ;
		GetAcceleration(Spatial, dAccelerationX, dAccelerationY, dAccelerationZ);
		printf("X Input local Accerleration: %G\n", dAccelerationX);
		printf("Y Input local Accerleration: %G\n", dAccelerationY);
		printf("Z Input local Accerleration: %G\n", dAccelerationZ);

		double dResultantAcceleration = GetdResultantAcceleration(dAccelerationX, dAccelerationY, dAccelerationZ);
		printf("Overall acceralertion%G\n", dResultantAcceleration);

		if ((dResultantAcceleration < 1.081)&(dResultantAcceleration > 0.891)) {
			GetAngularPos(dAngleX, dAngleY, dAngleZ, dAccelerationX, dAccelerationY, dAccelerationZ);
		}
		printf("X Angle: %d\n", (int)dAngleX);
		printf("Y Angle: %d\n", (int)dAngleY);
		printf("Z Angle: %d\n", (int)dAngleZ);
		fprintf(gnuplotPipe, "%G ", dAccelerationX);
		AccelerationGravityCorrection(dAccelerationX, dAccelerationY, dAccelerationZ, dAngleX, dAngleY, dAngleZ);
		//Zeros very small accelerations
		if ((dAccelerationX<0.025) & (dAccelerationX>-0.025)) {
			dAccelerationX = 0;
		}
		if ((dAccelerationY<0.025) & (dAccelerationY>-0.025)) {
			dAccelerationY = 0;
		}
		if ((dAccelerationZ<0.025) & (dAccelerationZ>-0.025)) {
			dAccelerationZ = 0;
		}
		fprintf(gnuplotPipe, "%G\n", dAccelerationX);
		printf("X Gravity Negated Accerleration: %G\n", dAccelerationX);
		printf("Y Gravity Negated Accerleration: %G\n", dAccelerationY);
		printf("Z Gravity Negated Accerleration: %G\n", dAccelerationZ);

		AccelerationCorrection(dAccelerationX, dAccelerationY, dAccelerationZ, dAngleX, dAngleY, dAngleZ, dGlobalAccelerationZ);

		/*GetMagneticField(Spatial,dMagneticFieldX,dMagneticFieldY,dMagneticFieldZ);
		GetAngularPosMagnetic(dMagAngleX, dMagAngleY, dMagAngleZ, dMagneticFieldX, dMagneticFieldY, dMagneticFieldZ);
		//printf("X MagAngle: %d\n", (int)dMagAngleX);
		//printf("Y MagAngle: %d\n", (int)dMagAngleY);
		//printf("Z MagAngle: %d\n", (int)dMagAngleZ);

		AccelerationCorrectionMag(dAccelerationX, dAccelerationY, dAccelerationZ, dMagAngleX, dMagAngleY, dMagAngleZ, dGlobalAccelerationNS);
		printf("NS:%G\n", dGlobalAccelerationNS);*/

		AccelerationCorrectionX(dAccelerationX, dAccelerationY, dAccelerationZ, dAngleX, dAngleY, dAngleZ, dGlobalAccelerationX);

		clock_t end = clock();
		float fCurrentLoopPeriod = ((float)(end - start) / CLOCKS_PER_SEC);

		dGlobalVelocityZ += (dGlobalAccelerationZ / fCurrentLoopPeriod);
		//dGlobalVelocityNS += (dGlobalAccelerationNS / fCurrentLoopPeriod);
		dGlobalVelocityX += (dGlobalAccelerationX / fCurrentLoopPeriod);

		//Zeros velocity if it changes little over five samples
		if ((dGlobalVelocityZ<dGlobalVelocityZBuffer + 3)&(dGlobalVelocityZ>dGlobalVelocityZBuffer - 3)) {
			iZeroVelocityCounter++;
			if (iZeroVelocityCounter > 5) {
				dGlobalVelocityZ = 0;
			}
		}
		else {
			iZeroVelocityCounter = 0;
		}

		//Zeros velocity if it changes little over five samples
		if ((dGlobalVelocityX<dGlobalVelocityXBuffer + 5)&(dGlobalVelocityX>dGlobalVelocityXBuffer - 5)) {
			iZeroVelocityCounterX++;
			if (iZeroVelocityCounterX > 5) {
				dGlobalVelocityX = 0;
			}
		}
		else {
			iZeroVelocityCounterX = 0;
		}

		/*if ((dGlobalVelocityNS<dGlobalVelocityNSBuffer + 2)&(dGlobalVelocityNS>dGlobalVelocityNSBuffer - 2)) {
		iZeroVelocityCounterNS++;
		if (iZeroVelocityCounterNS > 5) {
		dGlobalVelocityNS = 0;
		}
		}
		else {
		iZeroVelocityCounterNS = 0;
		}*/
		dGlobalVelocityZBuffer = dGlobalVelocityZ;
		WandPosChangeY = dGlobalVelocityZBuffer;
		dGlobalVelocityXBuffer = dGlobalVelocityX;
		WandPosChangeX = dGlobalVelocityXBuffer;
		//WandPosChangeX = dGlobalVelocityX;
		//dGlobalVelocityNSBuffer = dGlobalVelocityNS;
		//WandPosChangeX = dGlobalVelocityNSBuffer;

		//Limits cursor change values
		if (WandPosChangeY > 150) {
			WandPosChangeY = 150;
		}
		if (WandPosChangeY < -150) {
			WandPosChangeY = -150;
		}
		if (WandPosChangeX > 150) {
			WandPosChangeX = 150;
		}
		if (WandPosChangeX < -150) {
			WandPosChangeX = -150;
		}
		//Scales the cursor change values
		WandPosChangeY *= 0.75;
		WandPosChangeX *= 0.75;

		//printf("NSVelocity:%G\n", dGlobalVelocityNS);
		printf("ZVelocity:%G\n", dGlobalVelocityZBuffer);
		//printf("NSVelocity:%G\n", dGlobalVelocityNSBuffer);
		printf("XVelocity:%G\n", dGlobalVelocityXBuffer);

		dRunTime += fCurrentLoopPeriod;
		printf("%G ", dRunTime);
		printf("Y:%G\n", WandPosChangeY);
		printf("X:%G\n", WandPosChangeX);

		//Sends data to gnuplot
		fprintf(gnuplotPipe, "%G ", dRunTime);
		fprintf(gnuplotPipe, "%G\n", dAccelerationX);
		//fprintf(gnuplotPipe, "%G\n", dAngleY);
		//fprintf(gnuplotPipe, "%G\n", dGlobalVelocityZ);
		//fprintf(gnuplotPipe, "%G\n", dAngleY);
		//fprintf(gnuplotPipe, "%G\n", WandPosChangeX);
		///fprintf(gnuplotPipe, "%G\n", dGlobalVelocityNSBuffer);
		///fflush(gnuplotPipe);

		//Controls the mouse cursor
		double dScaleFactorX;
		double dScaleFactorY;
		int CursorPosX;
		int CursorPosY;

		Screen_GetScaleFactor(dScaleFactorX, dScaleFactorY);
		Mouse_GetPos(CursorPosX, CursorPosY);

		CursorPosX = (int)(((double)CursorPosX + WandPosChangeX) * dScaleFactorX);
		CursorPosY = (int)(((double)CursorPosY + WandPosChangeY) * dScaleFactorY);
		Mouse_SetPos(CursorPosX, CursorPosY);
		//DetectClick(dScaleFactorX,dScaleFactorY);

		if (dRunTime > 120) {
			break;
		}
	}
	//Closes the gnuplot pipe
	fputs("e\n", gnuplotPipe);
	_pclose(gnuplotPipe);
	return 0;
}

