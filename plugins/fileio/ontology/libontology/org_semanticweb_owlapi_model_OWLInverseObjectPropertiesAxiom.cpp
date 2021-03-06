#include <string.h>
#include <jni.h>
#include <java_marker.h>
#include <java_array.h>
#include <org_semanticweb_owlapi_model_OWLInverseObjectPropertiesAxiom.h>
#include <java_lang_Object.h>
#include <org_semanticweb_owlapi_model_OWLNaryPropertyAxiom.h>
#include <org_semanticweb_owlapi_model_OWLObjectPropertyAxiom.h>
#include <java_util_Set.h>
#include <org_semanticweb_owlapi_model_OWLObjectPropertyExpression.h>
extern JNIEnv     *javaEnv;
extern JavaMarker *wrapperIntern;

namespace org {
namespace semanticweb {
namespace owlapi {
namespace model {

OWLInverseObjectPropertiesAxiom::OWLInverseObjectPropertiesAxiom(JavaMarker* dummy)
  : java::lang::Object(dummy)
{
  updateAllVariables(wrapperIntern);
}

OWLInverseObjectPropertiesAxiom::OWLInverseObjectPropertiesAxiom(jobject obj)
  : java::lang::Object(obj)
{
  updateAllVariables(wrapperIntern);
}

void OWLInverseObjectPropertiesAxiom::updateAllVariables(JavaMarker* dummy)
{
  jclass cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
}

void OWLInverseObjectPropertiesAxiom::updateAllNonFinalVariables(JavaMarker* dummy)
{
  jclass cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
}

org::semanticweb::owlapi::model::OWLInverseObjectPropertiesAxiom* OWLInverseObjectPropertiesAxiom::getAxiomWithoutAnnotations()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "getAxiomWithoutAnnotations", "()Lorg/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom;");
  handleJavaException(wrapperIntern);
  jobject jresult=javaEnv->CallObjectMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  org::semanticweb::owlapi::model::OWLInverseObjectPropertiesAxiom* result;
  if (jresult!=NULL) {
    result=new org::semanticweb::owlapi::model::OWLInverseObjectPropertiesAxiom(jresult);
    javaEnv->DeleteLocalRef(jresult);
  } else {
    result=NULL;
  }
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}

java::util::Set* OWLInverseObjectPropertiesAxiom::asSubObjectPropertyOfAxioms()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "asSubObjectPropertyOfAxioms", "()Ljava/util/Set;");
  handleJavaException(wrapperIntern);
  jobject jresult=javaEnv->CallObjectMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  java::util::Set* result;
  if (jresult!=NULL) {
    result=new java::util::Set(jresult);
    javaEnv->DeleteLocalRef(jresult);
  } else {
    result=NULL;
  }
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}

org::semanticweb::owlapi::model::OWLObjectPropertyExpression* OWLInverseObjectPropertiesAxiom::getFirstProperty()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "getFirstProperty", "()Lorg/semanticweb/owlapi/model/OWLObjectPropertyExpression;");
  handleJavaException(wrapperIntern);
  jobject jresult=javaEnv->CallObjectMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  org::semanticweb::owlapi::model::OWLObjectPropertyExpression* result;
  if (jresult!=NULL) {
    result=new org::semanticweb::owlapi::model::OWLObjectPropertyExpression(jresult);
    javaEnv->DeleteLocalRef(jresult);
  } else {
    result=NULL;
  }
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}

org::semanticweb::owlapi::model::OWLObjectPropertyExpression* OWLInverseObjectPropertiesAxiom::getSecondProperty()
{
  jclass    cls = javaEnv->FindClass("org/semanticweb/owlapi/model/OWLInverseObjectPropertiesAxiom");
  handleJavaException(wrapperIntern);
  jmethodID mid = javaEnv->GetMethodID(cls, "getSecondProperty", "()Lorg/semanticweb/owlapi/model/OWLObjectPropertyExpression;");
  handleJavaException(wrapperIntern);
  jobject jresult=javaEnv->CallObjectMethod(this->getJavaObject(), mid);
  handleJavaException(wrapperIntern);
  org::semanticweb::owlapi::model::OWLObjectPropertyExpression* result;
  if (jresult!=NULL) {
    result=new org::semanticweb::owlapi::model::OWLObjectPropertyExpression(jresult);
    javaEnv->DeleteLocalRef(jresult);
  } else {
    result=NULL;
  }
  return result;
  updateAllNonFinalVariables(wrapperIntern);
}
}
}
}
}
