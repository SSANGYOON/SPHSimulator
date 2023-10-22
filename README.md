# SPHSimulator

## 주제 : SPH(smoothed particle hydrodynamics) 알고리즘을 이용한 유체 시뮬레이션

## 목적 :
### 1. GPU상에서 작동하는 SPH 알고리즘을 이용한 유체 시뮬레이션
### 2. GUI를 통해서 유체 발생 위치 및 장애물 배치 기능 구현
### 3. 투명 유체의 렌더링 파이프라인 작성

## 계획:
#### 1주차 : Directx11 + imgui를 이용한 개발환경 구성 및 SPH 개념 학습

#### 2주차 : Opengl + cpu 기반의 프로젝트( https://github.com/lijenicol/SPH-Fluid-Simulator )를 Directx + GPU기반 프로젝트로 작성

#### 3 ~ 5주차 : GUI를 이용한 유체 특성(입자 크기, 점성 등)조절 기능, 장애물 배치, 유체 생성 위치 조정 기능 구현

#### 6 ~ 8주자 : 투명 유체의 랜더링 파이프라인 작성

## 진행상황:
### 1주차
#### ~10/13 : 
* 개발환경 구성 + 및 SPH 개념 학습 

### 2주차
#### 10/16 :
* SPH에 필요한 GPU에서 작동하는 Sorting 알고리즘 구현 시작
* 
#### SPH에 Sorting을 사용하는 이유
* SPH에서 입자들을 공간을 일정크기로 나눈 격자에 배치된다고 가정하고 각 입자들은 자신이 속한 격자와 그 인접한 격자에 속한 입자들하고만 상호작용 한다고 가정
* 가령 X,Y,Z 격자에 속한 입자는 X±1,Y±1,Z±1 범위내의 격자에 속한 입자하고만 상호작용한다고 가정(그 밖의 입자와의 상호작용은 매우작아서 무시)
* 총 27개 격자의 파티클만 신경쓰면 된다.
* 입자들의 배열을 자신들의 해시값에 따라 정렬하면 같은 해시값의 입자들이 각 배열에 모여있게 됨
* 이 때 특정 해시값의 입자의 인덱스가 몇번부터 시작하는지를 저장해두면 어느 특정 격자의 입자들의 목록을 쉽게 찾을 수 있음
* 예를들어 27이라는 해시값의 입자들의 인덱스가 1080번 부터라고 하면 1080번 부터 계속 상호작용을 계산하면서 그 다음 인덱스의 파티클과의 상호작용을 계산한다.
* 해시값이 27이 아닌 입자의 인덱스가 나오면 그 격자에 대한 계산이 완료됨

#### 10/17 :
* Counting sort 알고리즘 구현(파티클이 속한 셀에 있는 다른 파티클들을 찾기 위해 모든 파티클들의 배열을 해시값으로 정렬)
* 간단한 파티클의 속성을 결정하는 UI 구현(기능 미구현)

<img src="./Images/ParticlePropertyUI.png">

#### 10/18 : 
* 파티클을 GPU에서 해시값으로 정렬, 정렬된 해시값에서 이웃한 파티클 목록를 구하는 기능 구현

#### 10/19
* 인스턴싱을 이용한 파티클 렌더링 쉐이더 코드 작성
<img src="./Images/ParticleInstanced.png">

* 파티클을 GPU에서 업데이트 하는 코드 구현 완료
<img src="./Images/ParticleFall.gif">

* 파티클 Sorting을 Bitonic Sort로 사용하기로 함
* Morton을 이용한 이웃 파티클 탐지 방법도 고려해보기로 함

#### 10/20
* 파티클 해시값 기반 정렬 알고리즘을 Counting sort에서 Inplace Sort가 가능한 Bitonic Sort로 변경
* 마우스 입력을 통한 카메라 줌인, 줌 아웃 및 카메라 회전 기능 구현
* 키 입력을 통한 시뮬레이션 시작 정지 기능 구현
* 적은 수의 큰 입자들을 긴 deltatime으로 시뮬레이션하면 마치 폭발하듯 튀는 것을 확인함 일단은 고정된 deltatime(0.003s)로 시뮬레이션

## 참고문헌 :

(SPH Fluids in Computer Graphics)[https://cg.informatik.uni-freiburg.de/publications/2014_EG_SPH_STAR.pdf]

(Particle-Based Fluid Simulation for Interactive Applications)[https://matthias-research.github.io/pages/publications/sca03.pdf]
