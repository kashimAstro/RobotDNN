#!/bin/sh

echo "Downloading shape_predictor_68_face_landmarks.dat"
curl -L -o model/shape_predictor_68_face_landmarks.dat.bz2 --progress-bar https://sourceforge.net/projects/dclib/files/dlib/v18.10/shape_predictor_68_face_landmarks.dat.bz2

echo "Extracting shape_predictor_68_face_landmarks.dat"
bzip2 -d model/shape_predictor_68_face_landmarks.dat.bz2

echo "Downloading dlib_face_recognition_resnet_model_v1.dat"
curl -L -o model/dlib_face_recognition_resnet_model_v1.dat.bz2 --progress-bar http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2

echo "Extracting dlib_face_recognition_resnet_model_v1.dat"
bzip2 -d model/dlib_face_recognition_resnet_model_v1.dat.bz2

echo "Downloading resnet34_1000_imagenet_classifier.dnn"
curl -L -o model/resnet34_1000_imagenet_classifier.dnn.bz2 --progress-bar http://dlib.net/files/resnet34_1000_imagenet_classifier.dnn.bz2

echo "Extracting resnet34_1000_imagenet_classifier.dnn"
bzip2 -d model/resnet34_1000_imagenet_classifier.dnn.bz2

echo "Done"
