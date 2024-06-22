# signTranslateMachine

## Overview

이 repository는 Ajou University System Programming & practice 수업의 Team 7이 개발한 “Sign Translation Machine” 프로젝트의 코드와 문서를 포함하고 있습니다. 이 프로젝트의 주요 목표는 Smart Glove를 사용하여 수화(수어)를 실시간으로 한국어 문자로 번역하여 청각 장애인과 청인 간의 의사소통을 향상시키는 것입니다.

## Team Member

| Name | Dept | Github | 
| --- | --- | --- |
| 정용훈 | Software & Computer Engineering | https://github.com/hun9008 |
| 남현원 | Software & Computer Engineering | https://github.com/hwnam5 |
| 이지수 | Digital Media | https://github.com/LJSoo99 |
| 전 진 | Software & Computer Engineering | https://github.com/jeenie2727 |

## Project Motivation

이 프로젝트는 수화가 있음에도 불구하고 청각 장애인이 겪는 의사소통 문제에서 출발했습니다. 본 프로젝트는 이러한 격차를 해소하여, 광범위한 학습이나 제3자 통역 없이 간단한 의사소통 방법을 제공하는 것을 목표로 합니다. 다른 프로젝트와 달리, 이 프로젝트는 한국 수어와 그 고유한 특징을 타겟으로 합니다.

## Project Goals

주요 목표는 청각 장애인과 청인 간의 일상적인 의사소통을 개선하는 것입니다. 다양한 센서를 사용하여 손 움직임과 손가락의 굽힘을 정확하게 감지함으로써 수어를 한국어 문자로 번역하여 원활한 의사소통을 가능하게 하고, 청각 장애인의 사회 참여와 독립성을 향상시키는 것을 목표로 합니다.

## System Overview

시스템은 Raspberry Pi를 이용하여 네 가지 주요 구성 요소로 나누어집니다:

	1. Glove (RPI1): 플렉스 및 기울기 센서를 사용하여 수화 입력을 캡처합니다.

	2. Control (RPI2): 모드 전환(초성, 중성, 종성) 및 오류 감지를 처리합니다.

	3. Core (RPI4): 입력 데이터를 처리하고 한국어 문자로 매핑하여 출력으로 전송합니다.

	4. Output (RPI3): 번역된 한국어 문자를 LCD에 표시합니다.

## Algorithm

RPI1: Glove Pi

	•	bcm2835 SPI 라이브러리를 사용하여 센서 데이터를 읽습니다.
	•	플렉스 센서 데이터를 이진 값으로 변환합니다.
	•	소켓을 통해 이진 데이터를 코어 Raspberry Pi로 전송합니다.

RPI2: Control Pi

	•	버튼과 LED를 사용하여 오류 감지 및 모드 전환을 구현합니다.
	•	모드 및 완료 신호를 코어 Raspberry Pi로 전송합니다.

RPI3: Output Pi

	•	사용자 정의 문자를 사용하여 한국어 문자를 LCD에 표시합니다.
	•	미리 정의된 구조와 배열을 사용하여 문자 매핑 및 표시를 처리합니다.

RPI4: Core Pi

	•	다른 모듈 간의 통신을 관리하는 중앙 서버 역할을 합니다.
	•	이진 입력 데이터를 처리하고 한국어 문자로 매핑합니다.
	•	처리된 데이터를 출력 모듈로 전송하고 오류를 보고합니다.

## Demo

[![프로젝트 시연 영상](https://img.youtube.com/vi/tprs2gadxFQ/0.jpg)](https://youtu.be/tprs2gadxFQ)

## Installation

프로젝트를 설정하려면 이 저장소를 클론하고 각 Raspberry Pi 모듈에 대한 설치 지침을 따르십시오. 모든 필수 라이브러리와 종속성이 설치되어 있는지 확인하십시오.

## Usage

    1.	Wear the Glove: Ensure all sensors are properly connected.

	2.	Start the Control Module: Use the control buttons to set the input mode (initial, medial, final consonant).

	3.	Run the Core Module: This module should be continuously running to process inputs.

	4.	Check the Output: View the translated characters on the LCD screen.

## Contribute

이 저장소를 포크하고, 이슈를 제출하고, 풀 리퀘스트를 만들어 기여하십시오. Sign Translation Machine의 기능과 사용성을 향상시키는 기여를 환영합니다.

## License

이 프로젝트는 MIT 라이선스에 따라 라이선스가 부여되었습니다. 자세한 내용은 LICENSE 파일을 참조하십시오.

자세한 문서와 기술 사양은 이 저장소의 docs 디렉토리에 제공된 프로젝트 보고서를 참조하십시오.

## Example

```
// glove
gcc -o glove glove.c -lbcm2835 -lpthread
sudo ./glove.c
```

```
// LCD
gcc -o lcd lcd_client.c -lwiringPi -lpthread
```
