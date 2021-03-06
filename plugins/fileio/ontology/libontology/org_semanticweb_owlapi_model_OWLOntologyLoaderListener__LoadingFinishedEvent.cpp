#include <string.h>
#include <jni.h>
#include <java_marker.h>
#include <java_array.h>
#include <org_semanticweb_owlapi_model_OWLOntologyLoaderListener__LoadingFinishedEvent.h>
#include <org_semanticweb_owlapi_model_OWLOntologyLoaderListener__LoadingEvent.h>
#include <org_semanticweb_owlapi_model_OWLOntologyID.h>
#include <org_semanticweb_owlapi_model_IRI.h>
#include <org_semanticweb_owlapi_model_OWLOntologyCreationException.h>
extern JNIEnv     *javaEnv;
extern JavaMarker *wrapperIntern;

namespace org {
namespace semanticweb {
namespace owlapi {
namespace model {

OWLOntologyLoaderListener__LoadingFinishedEvent::OWLOntologyLoaderListener__LoadingFinishedEvent(JavaMarker* dummy)
  : org::semanticweb::owlapi::model::OWLOntologyLoaderListener__LoadingEvent(dummy)
{
  updateAllVariables(wrapperIntern);
}

OWLOntologyLoaderListener__LoadingFinishedEvent::OWLOntologyLoaderListener__LoadingFinishedEvent(jobject obj)
  : org::semanticweb::owlapi::model::OWLOntologyLoaderListener__LoadingEvent(obj)
{
  updateAllVariables(wrapperIntern);
}

OWLOntologyLoaderListener__LoadingFinishedEvent::OWLOntologyLoaderListener__LoadingFinishedEvent(org::semanticweb::owlapi::model::OWLOntologyID* arg1, org::semanticweb::owlapi::model::IRI* arg2, bool arg3, org::semanticweb::owlapi::model::OWLOntologyCreationException* arg4)
  : org::semanticweb::owlapi::model::OWLOntologyLoaderListener__LoadingEvent(wrapperIntern)
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLOntologyLoaderListener$LoadingFinishedEvent");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "<init>", "(Lorg/semanticweb/owlapi/model/OWLOntologyID;Lorg/semanticweb/owlapi/model/IRI;ZLorg/semanticweb/owlapi/model/OWLOntologyCreationException;)V");
  handleJavaException(wrapperIntern);
  jobject jarg1;
  if (arg1!=NULL) {
    jarg1=arg1->getJavaObject();
  } else {
    jarg1=NULL;
  }
  jobject jarg2;
  if (arg2!=NULL) {
    jarg2=arg2->getJavaObject();
  } else {
    jarg2=NULL;
  }
  jboolean jarg3 = (jboolean)arg3;
  jobject jarg4;
  if (arg4!=NULL) {
    jarg4=arg4->getJavaObject();
  } else {
    jarg4=NULL;
  }
  jobject o = javaEnv->NewObject(cls, mid, jarg1, jarg2, jarg3, jarg4);
  handleJavaException(wrapperIntern);
  setJavaObject(o);
  javaEnv->DeleteLocalRef(o);
  updateAllVariables(wrapperIntern);
}

void OWLOntologyLoaderListener__LoadingFinishedEvent::updateAllVariables(JavaMarker* dummy)
{
  jclass cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLOntologyLoaderListener$LoadingFinishedEvent");
  handleJavaException(wrapperIntern);
}

void OWLOntologyLoaderListener__LoadingFinishedEvent::updateAllNonFinalVariables(JavaMarker* dummy)
{
  jclass cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLOntologyLoaderListener$LoadingFinishedEvent");
  handleJavaException(wrapperIntern);
}

org::semanticweb::owlapi::model::OWLOntologyCreationException* OWLOntologyLoaderListener__LoadingFinishedEvent::getException()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLOntologyLoaderListener$LoadingFinishedEvent");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "getException", "()Lorg/semanticweb/owlapi/model/OWLOntologyCreationException;");
  handleJavaException(wrapperIntern);
  jobject jresult=javaEnv->CallObjectMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  org::semanticweb::owlapi::model::OWLOntologyCreationException* result;
  if (jresult!=NULL) {
    result=new org::semanticweb::owlapi::model::OWLOntologyCreationException(jresult);
    javaEnv->DeleteLocalRef(jresult);
  } else {
    result=NULL;
  }
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}

bool OWLOntologyLoaderListener__LoadingFinishedEvent::isSuccessful()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLOntologyLoaderListener$LoadingFinishedEvent");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "isSuccessful", "()Z");
  handleJavaException(wrapperIntern);
  jboolean jresult=javaEnv->CallBooleanMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  bool result = (bool)jresult;
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}
}
}
}
}
