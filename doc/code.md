## Code structure and organization.

This code tries to simulate a cosmic ray detector organized in two layers, the bottom one with 90 scintillator bars of 1350 cm and the upper one with 60 scintillator bars of 900 cm. Both with an area of 14x14mm^2.

This detector is intended for recovering the tracks of the detected cosmic rays and check a possible impact in a qpu chip above the detector.

To organice the code, the structure goes as follows (the general implementation is based on tutorials [1], [2] from Mustafa Schmidt): 

1. Generated particles.

To simulate the incident cosmic rays we use CRY generator, the implementation of this software is done within geant4, this covers the classes ```PrimaryGeneratorCRY```, ```RNGWrapper``` to generate the random numbers needed by this software and ```PrimaryGeneratorMessenger``` to let the user update diferent CRY settings for the generation of the particles. The code is based on Alf Gook program [3]. In this last file another two commands are implemented to allow the <code style="color : red">generation of a csv file</code> with the data from the generated particles, to check they are what we expect, and a second one to allow debug prints for the code. 

2. Detector construction.

The idea to construct the detector is conditioned to the generation of the photons within the scintillator bars. There are two options which the code tries to implement to generate the optical photons after a cosmic ray impacts the bar and ionizes the medium. The first one uses geant4 bult-in methods to generate the photons, which are generated after defining the optical propierties of the materials [4]. Since this is a computationally demanding operation, a faster approach would be to generate the photons by parametrising their behaviour, assigning a group velocity determined by the refractive index of the medium. In this second option, the point at which the cosmic ray impacts the bar is set as the initial point for the generated photons, simulated with this assigned velocity, already described. This allows us to recreate the times at which the photons impact the extremes of the bar.

To implement both options, our approach was to introduce a messenger to allow the user to choose the method to generate the photons, in oder to provide a comparation to check the parametrized implementation. By doing this, we encounter the first problem: how to reinitialize the geometry without the need of compiling the simulation again.

Ultimatly, the goal of the simulation is to obtain the times at which the photons arrive to each extreme of the bar (this is needed to compute the impact point and recreate the cosmic ray trace, which is what we will do in the real world). In order to do that we need to declare some volumes as sensitive detectors. The implementation is orginized as follows, the volumes set as sensitive detectors are the SiPM, situated at the extremes of the bars, the scintillator bars themselves and the qpu (this last one is to check the reconstruction of the track for the cases an impact in the qpu occurs).


```QDDetectorConstruction.cc```

- GetSimulationMode
- SetSimulationMode
- CleanupGeometry
- UpdateGeometry
- DefineScintillatorMaterials 
- DefineMaterials
- ConstructScintillatorLayer1
- ConstructSicntillatorLayer2
- ConstructQPU
- DefineVolumes
- Construct
- ConstructSDandField


3. Particle detection. 

In order to detect the cosmic rays impacted we declared the sensitive detectors specified in the previous section. We have three sensitive volumes, and thus the easiest approach to follow is to save the relevant variables for each volume in three different files, however, if the output file is in root format this option is not available [5]. 

Another detail to have into account is that the output files would be conditioned by the mode of the simulation chosen, and the generated ntuples to be saved in the same root file are conditioned to the user choice, and should also change when the command defined in the DetectorMessenger class is called.




[1] https://www.youtube.com/watch?v=Lxb4WZyKeCE&list=PLLybgCU6QCGWgzNYOV0SKen9vqg4KXeVL
[2] https://www.youtube.com/watch?v=eG1cXevs0Gs&list=PLLybgCU6QCGUYAOwtyI4U8cRL6ig0p3c7
[3] https://github.com/alfgook/G4HpGeCoinc
[4] https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#optical-photon-processes
[5] https://geant4-ed-project.pages.in2p3.fr/geant4-ed-web/docs/analysis.pdf