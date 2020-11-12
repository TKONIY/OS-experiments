/**
 * ! 一共有四类人
 * male  : 男性顾客
 * female: 女性顾客
 * male_barber: 只会给男性理发的理发师, 简称男性理发师
 * both_barber: 可以给男性和女性理发的理发师, 简称全能理发师
 *
 * 逻辑挺清晰:
 * 1. 全局锁: 条件变量用的
 * 2. 四个不同的条件变量供四种人把自己挂起和把别人唤醒
 * 3. 四个整数统计四种人的人数, 主要用于判断, 理发师要判断有没有人
 * 有哪些人而叫醒顾客, 顾客也要经过判断来叫醒理发师, wait之前人数++,
 * notify之前人数--
 * 4. 四个notify变量记录某一类人是否曾经发出notify,
 * 被notify的人通过判断来确定自己是被谁叫醒的, 并减少这个数量
 * notify之前++, 从wait中醒来之后--;
 *
 * 后续可以改成lambda函数写法
 *
 */
#include <condition_variable>
#include <mutex>
#include <thread>

std::mutex global_mutex;                  // 全局锁
std::condition_variable female_customer;  // V表示告知一个女生
std::condition_variable male_customer;    // V表示告知一个男生
std::condition_variable both_barber;  // V表示叫醒一个全能的理发师
std::condition_variable male_barber;  // V表示叫醒一个男性理发师

int n_male = 0;         //等待中的男性顾客人数
int n_female = 0;       //等待中的女性理发师人数
int n_male_barber = 0;  //睡着的只能为男性理发的理发师人数
int n_both_barber = 0;  //睡着的全能理发师人数

int n_male_barber_notify = 0;  //发送了notify的男性理发师个数
int n_both_barber_notify = 0;  //发送了notify的全能理发师个数
int n_male_notify = 0;         //发送了notify的男性顾客个数
int n_female_notify = 0;       //发送了notify的女性顾客个数

const int n_chairs = 6;  //总共座椅数

/*********辅助函数********************/
void try_enter(const char* gender) {
  printf("%s 顾客尝试进入理发店\n", gender);
}
void enter(const char* gender) { printf("%s顾客进入理发店\n", gender); }

void leave(const char* gender) { printf("%s顾客离开理发店\n", gender); }

void barber(const char* barber_gender) {
  printf("%s理发师理发\n", barber_gender);
}
void barbered(const char* customer_gender) {
  printf("%s顾客被理发\n", customer_gender);
}

/***********************/

//男性客户
void MaleCustomer() {
  std::unique_lock<std::mutex> monitor(global_mutex);

  try_enter("男性");

  if (n_female + n_male < n_chairs          //有空椅子
      || (n_female + n_male < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {  //    且 随机决定后选择等待)

    enter("男性");

    if (n_male_barber > 0) {  // 如果有male理发师空闲
      n_male_barber--;        // 叫醒他, 睡着的人数--
      n_male_notify++;
      male_barber.notify_one();      // 叫醒他
    } else if (n_both_barber > 0) {  // 如果只有全能理发师空闲
      n_both_barber--;               // 叫醒他, 睡着的人数--
      n_male_notify++;
      both_barber.notify_one();  // 叫醒他
    } else {                     // 否则
      n_male++;
      while (n_male_barber_notify == 0 && n_both_barber_notify == 0)
        male_customer.wait(monitor);  //一直等到有理发师空闲把我叫醒吧
      if (n_male_barber_notify > 0) {
        n_male_barber_notify--;  //是male barber叫醒我的
      } else if (n_both_barber_notify > 0) {
        n_both_barber_notify--;  //是both barber叫醒我的
      }                          // else 出bug了有负数了
    }

    monitor.unlock();
    barbered("男性");

  } else {  //不留了, 溜

    monitor.unlock();
    leave("男性");
  }
}

//女性客户
void FemaleCustomer() {
  std::unique_lock<std::mutex> monitor(global_mutex);

  try_enter("女性");
  if (n_male + n_female < n_chairs          //有空椅子
      || (n_male + n_female < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {  //        且 随机决定后选择等待)

    enter("女性");

    if (n_both_barber > 0) {  //如果有全能理发师空闲
      n_both_barber--;        // 睡着的全能理发师--
      n_female_notify++;
      both_barber.notify_one();          //叫醒他
    } else {                             //否则
      n_female++;                        //等待的女性++
      while (n_both_barber_notify == 0)  //一直等到有全能理发师空闲
        female_customer.wait(monitor);
      n_both_barber_notify--;
    }

    monitor.unlock();
    barbered("女性");

  } else {
    monitor.unlock();
    leave("女性");
  }
}

//男性理发师
void MaleBarber() {
  while (true) {
    std::unique_lock<std::mutex> monitor(global_mutex);

    if (n_male > 0) {  //如果有男性在等待
      n_male--;        // 等待的male个数变少了
      n_male_barber_notify++;
      male_customer.notify_one();  // 让他进来剪发, 假设notify后是走自己的
    } else {                       // 否则
      n_male_barber++;             // 睡觉中的barber计数++
      while (n_male_notify == 0)    //一直睡到有男性顾客来
        male_barber.wait(monitor);  // 那就醒来的人负责--
      n_male_notify--;
    }

    monitor.unlock();
    barber("男性理发师");
  }
}

//全能理发师
void BothBarber() {
  while (true) {
    std::unique_lock<std::mutex> monitor(global_mutex);

    if (n_female > 0) {  //如果有female在等待
      n_female--;        //叫她进来,等待的个数减少了
      n_both_barber_notify++;
      female_customer.notify_one();  //把她叫进来
    } else if (n_male > 0) {         // 如果只有male在等待
      n_male--;                      //等待的个数变少了
      n_both_barber_notify++;
      male_customer.notify_one();  //那也叫进来
    } else {                       //如果没人等
      n_both_barber++;             //睡觉中的全能理发师个数++
      while (n_male_notify == 0 && n_female_notify == 0)  //一直睡到有任意顾客来
        both_barber.wait(monitor);
      if (n_female_notify > 0) {
        n_female_notify--;
      } else if (n_male_notify > 0) {
        n_male_notify--;
      }
    }

    monitor.unlock();
    barber("全能理发师");
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
  for (int i = 0; i < 15; ++i) {
    std::thread t1(FemaleCustomer);
    t1.detach();
    std::thread t2(MaleCustomer);
    t2.detach();
    std::thread t3(MaleCustomer);
    t3.detach();
    std::thread t4(MaleCustomer);
    t4.detach();
    std::thread t5(FemaleCustomer);
    t5.detach();
  }
  while (true);
  // thread析构之前必须使用detach或join
}
