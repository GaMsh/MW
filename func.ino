// LEDs functions
void tickInternal()
{
  int stateBack = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !stateBack);
}

void tickOffAll()
{
  ticker1.detach();
}
