#ifndef EVALUATION_H
#define EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif
extern sensor_response_t *active_response;

int cmd_eval_temp(int argc, char **argv);
int cmd_eval_humid(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* EVALUATION_H */