# SPHSimulator

## 주제 : SPH 알고리즘을 이용한 유체 시뮬레이션

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
개발환경 구성 + 및 SPH 개념 학습 

### 2주차
#### 10/16 :
SPH에 필요한 GPU에서 작동하는 Sorting 알고리즘 구현 시작

#### 10/17 :
parallel sort 알고리즘 구현(파티클이 속한 셀에 있는 다른 파티클들을 찾기 위해 모든 파티클들의 배열을 해시값으로 정렬)
간단한 파티클의 속성을 결정하는 간단한 UI 구현
<img src="./Images/ParticlePropertyUI.png">

#### 10/18 : 
파티클을 GPU에서 해시값으로 정렬, 정렬된 해시값에서 이웃한 파티클 목록를 구하는 기능 구현

## 참고문헌 :
(SPH Fluids in Computer Graphics)[https://cg.informatik.uni-freiburg.de/publications/2014_EG_SPH_STAR.pdf]
(Particle-Based Fluid Simulation for Interactive Applications)[https://matthias-research.github.io/pages/publications/sca03.pdf]