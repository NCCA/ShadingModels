# The subdir template is really useful when we have loads of projects
# to build, just add each project and the .pro file will be found
TEMPLATE = subdirs
SUBDIRS = ./Constant/Constant.pro \
./DirectionalLightASD/DirectionalLightASD.pro \
./DirectionalLightPointLight/DirectionalLightPointLight.pro \
./MultipleLights/MultipleLights.pro \
./MultiplePointLights/MultiplePointLights.pro \
./PerFragADS/PerFragADS.pro \
./PerVertexASD/PerVertexASD.pro \
./PerVertexDiffuse/PerVertexDiffuse.pro \
./PointLightASD/PointLightASD.pro \
./PointLightDiffuse/PointLightDiffuse.pro \
./TwoSidedASD/TwoSidedASD.pro\
./WireFrameDiscard/WireFrameDiscard.pro\
