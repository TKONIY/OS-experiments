// C++11使用mutex和条件变量实现, 类似管程语法
/**
 * 方案一: 男性顾客和女性顾客都叫醒全部理发师,
 * 理发师通过条件判断自己要不要上场,然后叫醒对应的顾客 方案二:
 * 男性顾客和女性顾客判断理发师的人数，然后看看叫醒哪个理发师
 */
#include <condition_variable>
#include <mutex>
#include <thread>

std::unique_lock<std::mutex> monitor;     // 全局锁,类似管程概念
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

//男性客户
void MaleCustomer() {
  monitor.lock();

  //进入理发店
  if (n_waiting < n_chairs          //有空椅子
      || (n_waiting < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {    //    且 随机决定后选择等待)
    n_waiting++;

    if (n_male_barber > 0) {         // 如果有male理发师空闲
      n_male_barber--;               // 叫醒他, 睡着的人数--
      male_barber.notify_one();      // 叫醒他
    } else if (n_both_barber > 0) {  // 如果只有全能理发师空闲
      n_both_barber--;               // 叫醒, 睡着的人数--
      both_barber.notify_one();      // 叫醒他
    } else {                         // 否则
      n_male++;                      //排队的男人又多了
      while (n_male_barber == 0 && n_both_barber == 0)
        male_customer.wait(monitor);  //一直等到有理发师空闲把我叫醒吧
    }

    n_waiting--;
    monitor.unlock();
    //理发
  } else {  //不留了, 溜
    monitor.unlock();
    //离开
  }
}
//女性客户
void FemaleCustomer() {
  monitor.lock();

  //进入理发店
  if (n_waiting < n_chairs          //有空椅子
      || (n_waiting < 2 * n_chairs  //或 (人数没超过双倍座椅
          && rand() % 2 == 0)) {    //    且 随机决定后选择等待)
    n_waiting++;

    if (n_both_barber > 0) {      //如果有全能理发师空闲
      n_both_barber--;            // 睡着的全能理发师--
      both_barber.notify_one();   //叫醒他
    } else {                      //否则
      n_female++;                 //我等待咯
      while (n_both_barber == 0)  //一直等到有全能理发师空闲
        female_customer.wait(monitor);
    }

    n_waiting--;
    monitor.unlock();
    //理发
  } else {
    monitor.unlock();
    //离开
  }
}

//男性理发师
void MaleBarber() {
  while (true) {
    monitor.lock();

    if (n_male > 0) {              //如果有男性在等待
      n_male--;                    // 等待的male个数变少了
      male_customer.notify_one();  // 让他进来剪发
    } else {                       // 否则
      n_male_barber++;             // 睡觉中的barber计数++
      while (n_male == 0)          //一直睡到有男性顾客来
        male_barber.wait(monitor);
    }

    monitor.unlock();
    //剪发
  }
}

//全能理发师
void BothBarber() {
  while (true) {
    monitor.lock();

    if (n_female > 0) {              //如果有female在等待
      n_female--;                    //叫她进来,等待的个数减少了
      female_customer.notify_one();  //把她叫进来
    } else if (n_male > 0) {         // 如果只有male在等待
      n_male--;                      //等待的个数变少了
      male_customer.notify_one();    //那也叫进来
    } else {                         //如果没人等
      n_both_barber++;               //睡觉中的全能理发师个数++
      while (n_male == 0 && n_female == 0)  //一直睡到有任意顾客来
        both_barber.wait(monitor);
    }

    monitor.unlock();
    //剪发
  }
}

int main() {
  // 2+1理发师组合
  std::thread t1(MaleBarber);
  std::thread t2(MaleBarber);
  std::thread t3(BothBarber);
  //无数个顾客组合
  while (1) {
    std::thread t1(FemaleCustomer);
    std::thread t2(MaleCustomer);
  }
}
