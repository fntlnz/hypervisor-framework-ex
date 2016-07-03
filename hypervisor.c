#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <Hypervisor/hv.h>

#define megabyte (1024UL * 1024)

int
vm_create() {
  return hv_vm_create(HV_VM_DEFAULT);
}

int
vm_map() {
  size_t size = 256 * megabyte;
  void *uva;
  uva = valloc(size);
  uint64_t gpa = 0;
  return hv_vm_map(uva, gpa, size, HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC);
}

static void *
thread_run(void *value_void_ptr) {
  hv_vcpuid_t vcpu;

  if (hv_vcpu_create(&vcpu, HV_VCPU_DEFAULT) != HV_SUCCESS) {
    fprintf(stderr, "error creating vcpu");
    return NULL;
  }


  if (hv_vcpu_run(vcpu) != HV_SUCCESS) {
    fprintf(stderr, "error hv_vcpu_run");
    return NULL;
  }

  /* Test write / read register */
  if (hv_vcpu_write_register(vcpu, HV_X86_RAX, 10) != HV_SUCCESS) {
    fprintf(stderr, "error writing register HV_X86_RAX");
    return NULL;
  }

  uint64_t *value_ptr = (uint64_t *) value_void_ptr;
  if (hv_vcpu_read_register(vcpu, HV_X86_RAX, value_ptr) != HV_SUCCESS) {
    fprintf(stderr, "error reading register HV_X86_RAX");
    return NULL;
  }

  if (hv_vcpu_destroy(vcpu) != HV_SUCCESS) {
    fprintf(stderr, "error destroying vcpu");
    return NULL;
  }

  return NULL;
}

void
add_cpu() {
  uint64_t value = 0;
  pthread_t thr;
  pthread_create(&thr, NULL, thread_run, &value);

  int error = pthread_join(thr, NULL);

  if (error) {
    fprintf(stderr, "error joining\n");
  }
  printf("value: %llu\n", value);
}

int
main(int argc, char *argv[]) {
  if (vm_create() != HV_SUCCESS) {
    fprintf(stderr, "error creating vm");
    return 1;
  }
  if (vm_map() != HV_SUCCESS) {
    fprintf(stderr, "error allocating memory");
    return 1;
  }
  add_cpu();
  return 0;

}
