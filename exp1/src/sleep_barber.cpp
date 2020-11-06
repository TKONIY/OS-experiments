// C++11使用mutex和条件变量实现, 类似管程语法
/**
 * C++11不符合stop and wait 和课本上的实现不一样,
 * 调用notify的时候并不会把自己挂起，
 * 而是执行完自己的剩余部分并unlock锁之后才会轮到被唤醒的线程执行
 * 所以当理发师唤醒女性顾客时, 女性顾客需要额外的信息判断是哪个理发师叫醒她,
 * 然后控制这个理发师的数量
 *
 * 针对这个特性总结出的原则, 对于while(p)wait();
 * 必须由被唤醒的进程修改p条件, 而不能在notify进程里执行
 */

#include <unistd.h>

#include <condition_variable>
#include <mutex>
#include <thread>

std::mutex global_mutex;                  // 全局锁
std::condition_variable female_customer;  // V表示告知一个女生
std::condition_variable male_customer;    // V表示告知一个男生
std::condition_variable both_barber;  // V表示叫醒一个全能的理发师
std::condition_variable male_barber;  // V表示侥幸一个男性理发师
int n_male = 0;                       //等待中的男性顾客人数
int n_female = 0;                     //等待中的女性理发师人数
int n_male_barber = 0;   //睡着的只能为男性理发的理发师人数
int n_both_barber = 0;   //睡着的全能理发师人数
const int n_chairs = 6;  //总共座椅数
int n_waiting = 0;       // waiting包含了在座椅和站着的人
enum { male, both, female } notify_by;  //记录哪个人发出了通知

void debug_status() {
  //上来记得加锁啊
  printf(
      "n_male=%d\nn_female=%d\n"
      /*\nn_male_barber=%d\nn_both_barber=%d\nn_chairs=%"
       "d\nn_waiting=%d\n"*/
      ,
      n_male, n_female /*, n_male_barber, n_both_barber, n_chairs,
      n_waiting*/);
}

void try_enter(const char* gender) {
  printf("%s 顾客尝试进入理发店\n", gender);
}
void enter(const char* gender) { printf("%s顾客进入理发店\n", gender); }

void leave(const char* gender) { printf("%s顾客离开理发店\n", gender); }

void barber(const char* barber_gender) {
  printf("%s理发师理发F\n", barber_gender);
}
void barbered(const char* customer_gender) {
  printf("%s顾客被理发\n", customer_gender);
}

//男性客户
void MaleCustomer() {
  sleep(1);
  std::unique_lock<std::mutex> monitor(global_mutex);

  try_enter("男性");

  if (n_waiting < n_chairs          //有空椅子
      || (n_waiting < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {    //    且 随机决定后选择等待)
    n_waiting++;

    enter("男性");

    n_male++;
    if (n_male_barber > 0) {  // 如果有male理发师空闲
      n_male_barber--;        // 叫醒他, 睡着的人数--
      notify_by = male;
      male_barber.notify_one();      // 叫醒他
    } else if (n_both_barber > 0) {  // 如果只有全能理发师空闲
      n_both_barber--;               // 叫醒他, 睡着的人数--
      notify_by = male;
      both_barber.notify_one();  // 叫醒他
    } else {                     // 否则
      printf("男性顾客等理发\n");
      while (n_male_barber == 0 && n_both_barber == 0)
        male_customer.wait(monitor);  //一直等到有理发师空闲把我叫醒吧
      printf("男性顾客停止等待\n");

      if (notify_by == male)
        n_male_barber--;  //是male barber叫醒我的
      else
        n_both_barber--;  //是both barber叫醒我的
    }

    n_waiting--;
    barbered("男性");
    monitor.unlock();
    //理发

  } else {  //不留了, 溜

    leave("男性");
    monitor.unlock();
    //离开
  }
}
//女性客户
void FemaleCustomer() {
  sleep(1);
  std::unique_lock<std::mutex> monitor(global_mutex);

  try_enter("女性");

  //进入理发店
  if (n_waiting < n_chairs          //有空椅子
      || (n_waiting < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {    //    且 随机决定后选择等待)
    n_waiting++;

    enter("女性");

    n_female++;               //等待的女性++
    if (n_both_barber > 0) {  //如果有全能理发师空闲
      n_both_barber--;        // 睡着的全能理发师--
      notify_by = female;
      both_barber.notify_one();  //叫醒他
    } else {                     //否则
      printf("女性顾客等理发\n");
      while (n_both_barber == 0)  //一直等到有全能理发师空闲
        female_customer.wait(monitor);
      printf("女性顾客停止等待\n");
      //一定是both叫醒我的 不用想了
      n_both_barber--;
    }

    n_waiting--;
    barbered("女性");
    monitor.unlock();
    //理发
  } else {
    leave("女性");
    monitor.unlock();
    //离开
  }
}

//男性理发师
void MaleBarber() {
  while (true) {
    std::unique_lock<std::mutex> monitor(global_mutex);

    // debug_status();

    n_male_barber++;   // 睡觉中的barber计数++
    if (n_male > 0) {  //如果有男性在等待
      n_male--;        // 等待的male个数变少了
      printf("nmale=%d 叫一个男性顾客进来\n", n_male);
      notify_by = male;  //表示是我叫醒的, 这里可以不要, 主要是对女生有用
      male_customer.notify_one();  // 让他进来剪发, 假设notify后是走自己的
    } else {                       // 否则
      printf("男性理发师睡了\n");
      while (n_male == 0)           //一直睡到有男性顾客来
        male_barber.wait(monitor);  // 那就醒来的人负责--
      n_male--;  //醒来的人负责减去条件相关的变量
      printf("男性理发师醒了\n");
    }

    // barber("男性理发师");
    monitor.unlock();
    //剪发
    // sleep(1);
  }
}

//全能理发师
void BothBarber() {
  while (true) {
    std::unique_lock<std::mutex> monitor(global_mutex);

    // debug_status();

    n_both_barber++;     //睡觉中的全能理发师个数++
    if (n_female > 0) {  //如果有female在等待
      n_female--;        //叫她进来,等待的个数减少了
      printf("nfemale=%d 叫一个女性顾客进来\n", n_female);
      notify_by = both;
      female_customer.notify_one();  //把她叫进来
    } else if (n_male > 0) {         // 如果只有male在等待
      n_male--;                      //等待的个数变少了
      printf("nmale=%d 叫一个男性顾客进来\n", n_male);
      notify_by = both;
      male_customer.notify_one();  //那也叫进来
    } else {                       //如果没人等
      printf("全能理发师睡了\n");
      while (n_male == 0 && n_female == 0)  //一直睡到有任意顾客来
        both_barber.wait(monitor);
      printf("全能理发师醒了\n");

      if (notify_by == male)
        n_male--;
      else
        n_female--;
    }

    // barber("全能理发师");
    monitor.unlock();
    //剪发
    // sleep(1);
  }
}

int main() {
  // 2+1理发师组合
  std::thread t1(MaleBarber);
  std::thread t2(MaleBarber);
  std::thread t3(BothBarber);

  t1.detach();
  t2.detach();
  t3.detach();

  // 无数个顾客组合
  while (1) {
    std::thread t1(FemaleCustomer);
    t1.detach();
    std::thread t2(MaleCustomer);
    t2.detach();
    std::thread t3(MaleCustomer);
    t3.detach();
    sleep(1);
  }
  // thread析构之前必须使用detach或join
}

// #include <unistd.h>

// #include <condition_variable>
// #include <mutex>
// #include <thread>
// std::mutex mutex;
// std::condition_variable cond;
// int a = 0;
// void t1() {
//   sleep(1);
//   std::unique_lock<std::mutex> lck(mutex);
//   a = 1;
//   cond.notify_one();
//   a = 0;
//   lck.unlock();
// }
// void t2() {
//   std::unique_lock<std::mutex> lck(mutex);
//   while (a == 0) cond.wait(lck);
//   lck.unlock();
// }
// int main() {
//   std::thread t22(t2);
//   std::thread t11(t1);
//   t11.join();
//   t22.join();
// }
