<?php

/**
 * @file
 * Payment method definition and helper functions.
 */

/**
 * Payment method callback: settings form.
 */
function commerce_collector_settings_form($settings = array()) {

  _commerce_collector_defaults($settings);

  $form['commerce_collector_login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collector credentials'),
  );
  $form['commerce_collector_login']['commerce_collector_environment'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#title' => t('Environment'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_environment'],
    '#options' => array(
      'https://ecommercetest.collector.se/v3.0/' => t('Test'),
      'https://ecommerce.collector.se/v3.0/' => t('Production'),
    ),
    '#description' => t('Choose which environment you want to use.'),
  );
  $form['commerce_collector_login']['commerce_collector_username'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Username'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_username'],
    '#description' => t('You should get this data from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_password'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Password'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_password'],
    '#description' => t('You should get this data from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_store_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Store ID'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_store_id'],
    '#description' => t('You should get this data from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_credit_terms'] = array(
    '#type' => 'textfield',
    '#title' => t('Absolute path to collector credit terms.'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_credit_terms'],
    '#required' => TRUE,
    '#description' => t('You should get link to online PDF document from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_secci'] = array(
    '#type' => 'textfield',
    '#title' => t('Absolute path to secci information.'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_secci'],
    '#required' => TRUE,
    '#description' => t('You should get link to online PDF document from Collector.'),
  );
  $form['commerce_collector_login']['commerce_collector_fee'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect an extra fee.'),
    '#description' => t('Check this box to specify an extra fee.'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_fee'],
  );
  $form['commerce_collector_login']['commerce_collector_fee_amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount of the extra fee.'),
    '#description' => t('Give the ammount you want to use if this method is selected. Be aware that this needs to be in minor units aks cents.'),
    '#default_value' => $settings['commerce_collector_login']['commerce_collector_fee_amount'],
    '#element_validate' => array('element_validate_number'),
    '#states' => array(
      'visible' => array(
        ':input[name="parameter[payment_method][settings][payment_method][settings][commerce_collector_login][commerce_collector_fee]"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['commerce_collector_invoice'] = array(
    '#type' => 'fieldset',
    '#title' => t('Invoice settings'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_country'] = array(
    '#type' => 'select',
    '#title' => t('Available countries'),
    '#required' => TRUE,
    '#multiple' => TRUE,
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_invoice_country'],
    '#options' => _commerce_collector_get_countries(),
    '#description' => t('This list has been made based on countries supported by Collector. Choose countries which are supported by your shop.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_delivery_address_profile'] = array(
    '#type' => 'select',
    '#title' => t('Delivery address profile'),
    '#required' => TRUE,
    '#multiple' => FALSE,
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_delivery_address_profile'],
    '#options' => _commerce_collector_get_customer_profile_instances(),
    '#description' => t('Which customer profile should be used as a delivery address.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_billing_address_profile'] = array(
    '#type' => 'select',
    '#title' => t('Billing address profile'),
    '#required' => FALSE,
    '#multiple' => FALSE,
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_billing_address_profile'],
    '#options' => _commerce_collector_get_customer_profile_instances(),
    '#description' => t('Which customer profile should be used as a billing address. If none - gets delivery address.'),
  );

  $form['commerce_collector_invoice']['commerce_collector_invoice_activation'] = array(
    '#type' => 'select',
    '#title' => t('Activation'),
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_invoice_activation'],
    '#options' => array(
      COMMERCE_COLLECTOR_INVOICE_ACTIVATION_MANUAL => t('Manual activation'),
    ),
    '#description' => t('Manual activation - Purchase will be preliminary and must be activated manually under Order Payment tab or through the Collector Partner Portal. You can also choose order states which will auto activate or cancel/credit payment for you.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_partial_payments'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#title' => t('Partial payments'),
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_invoice_partial_payments'],
    '#options' => _commerce_collector_get_rates(),
    '#description' => t('Select one of available invoice types. By default - invoice per order.'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_delivery'] = array(
    '#type' => 'select',
    '#title' => t('Delivery method'),
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_invoice_delivery'],
    '#options' => array(
      COMMERCE_COLLECTOR_INVOICE_DELIVERY_POST => t('Normal mail or in package'),
      COMMERCE_COLLECTOR_INVOICE_DELIVERY_EMAIL => t('E-mail'),
    ),
    '#description' => t('Print and send invoice in package or allow Collector to send it for you (if e-mail address has been provided). <strong>This value will be overwritten if customer will choose partial payments.</strong>'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_type'] = array(
    '#type' => 'select',
    '#title' => t('Invoice type'),
    '#default_value' => $settings['commerce_collector_invoice']['commerce_collector_invoice_type'],
    '#options' => array(
      COMMERCE_COLLECTOR_INVOICE_TYPE_DIRECT => t('Invoice in package or directly sent with e-mail'),
      COMMERCE_COLLECTOR_INVOICE_TYPE_MONTHLY => t('Monthly Invoice'),
      COMMERCE_COLLECTOR_INVOICE_TYPE_AGGREGATED => t('Aggregated Invoice'),
      COMMERCE_COLLECTOR_INVOICE_TYPE_INTEREST_FREE => t('Interest Free Invoice'),
      COMMERCE_COLLECTOR_INVOICE_TYPE_ANNUITY => t('Annuity invoice'),
    ),
    '#description' => t('Select one of available invoice types. By default - invoice per order. <strong>This value will be overwritten for partial payments, respectively Interest Free Invoice or Annuity invoice depends on partial payment value.</strong>'),
  );
  $form['commerce_collector_vat'] = array(
    '#type' => 'fieldset',
    '#title' => t('VAT settings'),
  );
  $form['commerce_collector_vat']['commerce_collector_use_vat_module'] = array(
    '#type' => 'select',
    '#title' => t('Use VAT module'),
    '#description' => t('If VAT module has been enabled you might want to use it instead of fixed VAT rate.'),
    '#default_value' => $settings['commerce_collector_vat']['commerce_collector_use_vat_module'],
    '#options' => array(
      TRUE => t('VAT module'),
      FALSE => t('Fixed VAT rate'),
    ),
  );
  $form['commerce_collector_vat']['commerce_collector_fixed_vat_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed VAT rate'),
    '#description' => t('ex. "22" for 22% or "15.5" for 15,5%'),
    '#size' => 6,
    '#maxlength' => 6,
    '#default_value' => $settings['commerce_collector_vat']['commerce_collector_fixed_vat_rate'],
    '#element_validate' => array('_commerce_collector_validate_decimal_number'),
    '#states' => array(
      'visible' => array(
        ':input[name="parameter[payment_method][settings][payment_method][settings][commerce_collector_vat][commerce_collector_use_vat_module]"]' => array('value' => 0),
      ),
    ),
  );
  return $form;
}

/**
 * Payment method callback: submit form.
 */
function commerce_collector_submit_form($payment_method, $pane_values, $checkout_pane, $order) {

  _commerce_collector_defaults($payment_method['settings']);

  if ($partial_payment_options = array_intersect_key(_commerce_collector_get_rates(), $payment_method['settings']['commerce_collector_invoice']['commerce_collector_invoice_partial_payments'])) {
    $form['partial_payment_option'] = array(
      '#type' => 'radios',
      '#required' => TRUE,
      '#default_value' => isset($order->data['commerce_collector']['pane_values']['partial_payment_option']) ? $order->data['commerce_collector']['pane_values']['partial_payment_option'] : NULL,
      '#title' => t('Choose partial payment option'),
      '#options' => $partial_payment_options,
    );
  }

  $form['personal_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Personal number'),
    '#default_value' => isset($order->data['commerce_collector']['pane_values']['personal_number']) ? $order->data['commerce_collector']['pane_values']['personal_number'] : NULL,
    '#required' => TRUE,
  );

  $form['confirmation'] = array(
    '#type' => 'checkbox',
    '#default_value' => isset($order->data['commerce_collector']['pane_values']['confirmation']) ? $order->data['commerce_collector']['pane_values']['confirmation'] : FALSE,
    '#title' => t('I accept <a href="@credit_terms">general terms and conditions for credit agreement</a> and <a href="@secci">standard european consumer credit information</a>.', array('@credit_terms' => $payment_method['settings']['commerce_collector_login']['commerce_collector_credit_terms'], '@secci' => $payment_method['settings']['commerce_collector_login']['commerce_collector_secci'])),
    '#required' => TRUE,
  );
  return $form;
}

/**
 * Payment method callback: submit form submission.
 */
function commerce_collector_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {

  // Adds nee line item with payment fee (only one no matter how many
  // transactions has been registered).
  commerce_collector_fee_line_item($payment_method, $order);

  $order->data['commerce_collector']['pane_values'] = $pane_values;
}

/**
 * Payment method callback: redirect form.
 */
function commerce_collector_redirect_form($form, &$form_state, $order, $payment_method) {

  // Reset all unprocessed transactions.
  _commerce_collector_cancel_deprecated_transactions($order);

  // Collector supports only full payments.
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  $charge = $order_wrapper->commerce_order_total->value();

  // Create and save transaction.
  $transaction = commerce_payment_transaction_new('commerce_collector', $order->order_id);
  $transaction->instance_id = $payment_method['instance_id'];
  $transaction->amount = $charge['amount'];
  $transaction->currency_code = $charge['currency_code'];
  $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;

  if (commerce_payment_transaction_save($transaction)) {
    $form['transaction'] = array('#type' => 'hidden', '#value' => $transaction->transaction_id);
    $form['#action'] = url('checkout/' . $order->order_id . '/payment/return/' . $order->data['payment_redirect_key']);
  }
  else {
    $form['#action'] = url('checkout/' . $order->order_id . '/payment/back/' . $order->data['payment_redirect_key']);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Proceed to Collector'),
  );

  return $form;
}

/**
 * Payment method callback: redirect form return validation.
 *
 * We trust Collector to be the only ones that know about this.
 */
function commerce_collector_redirect_form_validate($order, $payment_method) {

  try {
    $transaction = commerce_payment_transaction_load($_POST['transaction']);
    if (!is_object($transaction)) {
      throw new Exception('Missing transaction');
    }
  }
  catch (Exception $e) {
    commerce_payment_redirect_pane_previous_page($order);
    drupal_set_message(t('Missing transaction'), 'error');
    return FALSE;
  }

  try {
    // Load settings.
    $settings = $payment_method['settings'];
    _commerce_collector_defaults($settings);

    // Retrieve pane values from order.
    if (!isset($order->data['commerce_collector']['pane_values'])) {
      throw new Exception('Configuration error');
    }
    $pane_values = $order->data['commerce_collector']['pane_values'];

    $delivery_address_profile = $settings['commerce_collector_invoice']['commerce_collector_delivery_address_profile'];
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    $payload = array(
      'ActivationOption' => $payment_method['settings']['commerce_collector_invoice']['commerce_collector_invoice_activation'],
      'CountryCode' => $order_wrapper->{$delivery_address_profile}->commerce_customer_address->country->value(),
      'Currency' => $transaction->currency_code,
    );

    $payload['DeliveryAddress'] = array();
    $payload['DeliveryAddress']['Address1'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->thoroughfare->value();
    $payload['DeliveryAddress']['Address2'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->premise->value();
    $payload['DeliveryAddress']['COAddress'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->name_line->value();
    $payload['DeliveryAddress']['City'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->locality->value();
    $payload['DeliveryAddress']['CountryCode'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->country->value();
    $payload['DeliveryAddress']['PostalCode'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->postal_code->value();
    $payload['DeliveryAddress']['CellPhoneNumber'] = '';
    $payload['DeliveryAddress']['CompanyName'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->organisation_name->value();
    $payload['DeliveryAddress']['Email'] = $order->mail;
    $payload['DeliveryAddress']['Firstname'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->first_name->value();
    $payload['DeliveryAddress']['Lastname'] = $order_wrapper->{$delivery_address_profile}->commerce_customer_address->last_name->value();
    $payload['DeliveryAddress']['PhoneNumber'] = '';
    drupal_alter('commerce_collector_delivery_address', $payload['DeliveryAddress'], $payload, $order, $payment_method);

    $billing_address_profile = $settings['commerce_collector_invoice']['commerce_collector_billing_address_profile'];
    if (empty($billing_address_profile)) {
      $billing_address_profile = $delivery_address_profile;
    }
    $payload['InvoiceAddress'] = array();
    $payload['InvoiceAddress']['Address1'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->thoroughfare->value();
    $payload['InvoiceAddress']['Address2'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->premise->value();
    $payload['InvoiceAddress']['COAddress'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->name_line->value();
    $payload['InvoiceAddress']['City'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->locality->value();
    $payload['InvoiceAddress']['CountryCode'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->country->value();
    $payload['InvoiceAddress']['PostalCode'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->postal_code->value();
    $payload['InvoiceAddress']['CellPhoneNumber'] = '';
    $payload['InvoiceAddress']['CompanyName'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->organisation_name->value();
    $payload['InvoiceAddress']['Email'] = $order->mail;
    $payload['InvoiceAddress']['Firstname'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->first_name->value();
    $payload['InvoiceAddress']['Lastname'] = $order_wrapper->{$billing_address_profile}->commerce_customer_address->last_name->value();
    $payload['InvoiceAddress']['PhoneNumber'] = '';
    drupal_alter('commerce_collector_invoice_address', $payload['InvoiceAddress'], $payload, $order, $payment_method);

    // Partial payments support.
    if (!empty($pane_values['partial_payment_option'])) {
      $payload['ProductCode'] = $pane_values['partial_payment_option'];
      $payload['InvoiceDeliveryMethod'] = COMMERCE_COLLECTOR_INVOICE_DELIVERY_POST;
    }
    else {
      $payload['InvoiceDeliveryMethod'] = $settings['commerce_collector_invoice']['commerce_collector_invoice_delivery'];
    }

    // Add invoice rows.
    $invoice['InvoiceRows'] = array();
    foreach ($order_wrapper->commerce_line_items->getIterator() as $line) {
      $row = array(
        'ArticleId' => $line->line_item_label->value(),
        'Description' => commerce_line_item_title($line->value()),
        'Quantity' => $line->quantity->value(),
        'UnitPrice' => $line->commerce_unit_price->amount_decimal->value(),
        'VAT' => _commerce_collector_line_item_vat($payment_method, $line->commerce_unit_price->value()),
      );
      drupal_alter('commerce_collector_invoice_row', $payload['InvoiceAddress'], $payload, $order, $payment_method);
      $payload['InvoiceRows'][] = $row;
    }

    // Invoice type.
    $payload['InvoiceType'] = $settings['commerce_collector_invoice']['commerce_collector_invoice_type'];
    if (!empty($pane_values['partial_payment_option']) && in_array($pane_values['partial_payment_option'], array('IF_3_001', 'IF_6_001', 'IF_12_001'))) {
      // We have to overwrite default invoice type if user want to use interest
      // free invoice.
      $payload['InvoiceType'] = COMMERCE_COLLECTOR_INVOICE_TYPE_INTEREST_FREE;
    }
    elseif (!empty($pane_values['partial_payment_option']) && in_array($pane_values['partial_payment_option'], array('AN_24_001', 'AN_36_001'))) {
      // We have to overwrite default invoice type if user wants to use partial
      // payments with non-free interest rate.
      $payload['InvoiceType'] = COMMERCE_COLLECTOR_INVOICE_TYPE_ANNUITY;
    }

    // Set order number.
    $payload['OrderNo'] = $order->order_id;

    // Set order date.
    $payload['OrderDate'] = format_date($order->created, 'custom', 'Y-m-d\TH:i:sP');

    // Set personal number.
    $payload['RegNo'] = $pane_values['personal_number'];

    // Set store id.
    if ($store_id = $settings['commerce_collector_login']['commerce_collector_store_id']) {
      $payload['StoreId'] = $store_id;
    }

    // Save payload in transaction.
    $transaction->payload = $payload;

    // Send request.
    try {
      // Send request.
      $result = _commerce_collector_request($payload, $payment_method, 'AddInvoice');
    }
    catch (Exception $e) {
      if (isset($e->faultcode) && $message = _commerce_collector_fault_codes_messages($e->faultcode)) {
        // If error should be presented to customer.
      }
      else {
        // Otherwise.
        $message = t('Collector was not able to process payment for provided data. Choose another payment method.');
      }
      // Set message to end user.
      drupal_set_message($message, 'error');

      // Log original exception.
      watchdog_exception('commerce_collector', $e);

      // Throw good-looking exception ;).
      throw new Exception($message);
    }

    // Remote status has to be valid.
    if (!in_array($result->InvoiceStatus, array(COMMERCE_COLLECTOR_PAYMENT_STATUS_PENDING, COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED))) {
      throw new Exception(_commerce_collector_fault_codes_messages('DENIED_TO_PURCHASE'), 'DENIED_TO_PURCHASE');
    }

    $transaction->data['country_code'] = $payload['CountryCode'];
    $transaction->data['response'] = (array) $result;
    $transaction->remote_id = $result->InvoiceNo;
    $transaction->remote_status = $result->InvoiceStatus;

    // Change transaction status.
    commerce_collector_set_local_status($transaction);

    // Keep transaction status.
    if (!commerce_payment_transaction_save($transaction)) {
      throw new Exception('Unable to save transaction');
    }
  }
  catch (Exception $e) {
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    $transaction->message = '@message';
    $transaction->message_variables = array(
      '@message' => $e->getMessage(),
    );
    commerce_payment_transaction_save($transaction);

    commerce_payment_redirect_pane_previous_page($order);
    watchdog_exception('commerce_collector', $e);
    return FALSE;
  }
}

/**
 * Activate invoice for payment.
 */
function commerce_collector_activate_invoice($transaction) {

  // Load payment method instance to retrieve instance settings.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  // If invoice was already canceled.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED && $transaction->status == COMMERCE_PAYMENT_STATUS_SUCCESS) {
    return TRUE;
  }

  // Remote status was changed but local was outdated.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED) {
    commerce_collector_set_local_status($transaction);
    commerce_payment_transaction_save($transaction);
    return TRUE;
  }
  try {
    $data = array(
      'CountryCode' => $transaction->data['country_code'],
      'InvoiceNo' => $transaction->remote_id,
    );
    $result = _commerce_collector_request($data, $payment_method, 'ActivateInvoice');
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED;
    $transaction->data['response'] = (array) $result;
    commerce_collector_set_local_status($transaction);
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Invoice @number has been activated.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    if (isset($e->faultcode) && $e->faultcode == "s:INVOICE_NOT_FOUND") {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. You cannot change status of invoice which has been already rejected. Create new invoice instead.', array('@href' => url($uri['path']))), 'error');
    }
    else {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
    }
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Deactivate invoice for payment.
 */
function commerce_collector_deactivate_invoice($transaction) {

  // Load payment method instance to retrieve instance settings.
  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  // If invoice was already canceled.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED && $transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    return TRUE;
  }

  // Remote status was changed but local was outdated.
  if ($transaction->remote_status == COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED) {
    commerce_collector_set_local_status($transaction);
    commerce_payment_transaction_save($transaction);
    return TRUE;
  }
  try {
    $data = array(
      'CountryCode' => $transaction->data['country_code'],
      'InvoiceNo' => $transaction->remote_id,
    );
    // Try to deactivate invoice.
    $result = _commerce_collector_request($data, $payment_method, 'CancelInvoice');
    // If succeed - change transaction status.
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED;
    $transaction->data['response'] = (array) $result;
    commerce_collector_set_local_status($transaction);
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Invoice @number has been rejected.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    if (isset($e->faultcode) && $e->faultcode == "s:INVALID_INVOICE_STATUS") {
      // In case Invoice was activated before try credit it.
      return commerce_collector_credit_invoice($transaction);
    }
    else {
      drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
      watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
      return FALSE;
    }
  }
}

/**
 * Credit an invoice.
 */
function commerce_collector_credit_invoice($transaction) {

  $payment_method = commerce_payment_method_instance_load($transaction->instance_id);

  try {
    $data = array(
      'CountryCode' => $transaction->data['country_code'],
      'InvoiceNo' => $transaction->remote_id,
      'CreditDate' => format_date(REQUEST_TIME, 'custom', 'Y-m-d\TH:i:sP'),
    );
    $result = _commerce_collector_request($data, $payment_method, 'CreditInvoice');
    $transaction->remote_status = COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED;
    $transaction->data['invoice'] = (array) $result;
    commerce_collector_set_local_status($transaction);
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Invoice @number has been credited.', array('@number' => $transaction->remote_id)));
    return TRUE;
  }
  catch (Exception $e) {
    $uri = commerce_payment_ui_payment_transaction_uri($transaction);
    drupal_set_message(t('Unable to change status of <a href=@href>payment transaction</a>. Please contact administrator.', array('@href' => url($uri['path']))), 'error');
    watchdog('commerce_collector', print_r($e, TRUE), array(), WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Send request to Collector endpoint and get response.
 *
 * If you want to trigger this function wrap it with try..catch because it will
 * return Exception in case of any problems with request.
 *
 * @param array $data
 *   Array of arguments passed to soap call.
 * @param string $method
 *   Method name should be consistent with Collector specs.
 * @param string $wsdl
 *   WSDL filename corresponding to $method you use.
 * @param string $schema
 *   Name of SOAP schema corresponding to wsdl file.
 *
 * @return mixed
 *   Exception or result object.
 */
function _commerce_collector_request(array $data, $payment_method, $method, $wsdl = 'InvoiceServiceV32.svc', $schema = 'InvoiceService') {

  _commerce_collector_defaults($payment_method['settings']);

  $environment = $payment_method['settings']['commerce_collector_login']['commerce_collector_environment'];

  $client = new SoapClient($environment . $wsdl . '?singleWsdl', array(
    'trace' => TRUE,
    'features' => SOAP_SINGLE_ELEMENT_ARRAYS,
  ));

  $headers = array();
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'ClientIpAddress', $_SERVER['SERVER_ADDR']);
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'Username', $payment_method['settings']['commerce_collector_login']['commerce_collector_username']);
  $headers[] = new SoapHeader('http://schemas.ecommerce.collector.se/v30/' . $schema, 'Password', $payment_method['settings']['commerce_collector_login']['commerce_collector_password']);
  $client->__setSoapHeaders($headers);

  $result = $client->{$method}($data);

  return $result;
}

/**
 * Adds or removes fee line item from order.
 *
 * @param $payment_method
 *   Payment method instance object.
 * @param $order
 *   Order object.
 * @param bool $delete
 *   Boolean indicator if fee line item should be a part of order or not.
 *
 * @return bool
 *   Boolean status - TRUE if fee exists, FALSE if not.
 */
function commerce_collector_fee_line_item($payment_method, $order, $delete = FALSE) {

  if (!_commerce_collector_line_item_exists($order) && !$delete) {

    if ($payment_method['settings']['commerce_collector_login']['commerce_collector_fee']) {

      // Create the new line item.
      $line_item = commerce_line_item_new('collector_fee', $order->order_id);

      // Wrap the line item to simplify manipulating their field data.
      $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);

      // Populate the $line_item_wrapper...
      $line_item_wrapper->line_item_label = t('Invoice fee');
      $line_item_wrapper->quantity = 1;
      $line_item_wrapper->commerce_unit_price->amount = $payment_method['settings']['commerce_collector_login']['commerce_collector_fee_amount'];
      $line_item_wrapper->commerce_unit_price->currency_code = commerce_default_currency();

      // Set the price component of the unit price.
      $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add(
        $line_item_wrapper->commerce_unit_price->value(),
        'collector_fee',
        $line_item_wrapper->commerce_unit_price->value(),
        TRUE,
        FALSE
      );
      commerce_line_item_save($line_item);

      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      $order_wrapper->commerce_line_items[] = $line_item;
      return commerce_order_save($order);
    }
  }
  return FALSE;
}

/**
 * Change local status in reference to remote status.
 *
 * @param $transaction
 *   (Reference) Transaction object.
 */
function commerce_collector_set_local_status(&$transaction) {
  switch ($transaction->remote_status) {
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_ACTIVATED:
      $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
      $transaction->message = 'Invoice @number has been activated.';
      break;

    case COMMERCE_COLLECTOR_PAYMENT_STATUS_ON_HOLD:
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_PENDING:
    case COMMERCE_COLLECTOR_PAYMENT_STATUS_SIGNING:
      $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
      $transaction->message = 'Invoice @number has a pending status (@status) now';
      break;

    case COMMERCE_COLLECTOR_PAYMENT_STATUS_REJECTED:
      $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
      $transaction->message = 'Invoice @number has been rejected.';
      break;
  }
  $transaction->message_variables = array(
    '@number' => $transaction->remote_id,
    '@status' => $transaction->remote_status,
  );
}

/**
 * Calculate VAT for line item.
 *
 * Two methods available - VAT or Fixed Vat Rate (configurable with variable).
 */
function _commerce_collector_line_item_vat($payment_method, $unit_price) {
  $use_vat_module = $payment_method['settings']['commerce_collector_vat']['commerce_collector_use_vat_module'];
  // Use vat module based rate or fixed one.
  if (!$use_vat_module) {
    return $payment_method['settings']['commerce_collector_vat']['commerce_collector_fixed_vat_rate'];
  }
  // Check if price has components.
  if (!isset($unit_price['data']['components'])) {
    return 0;
  }
  // Get all VAT components.
  $components = $unit_price['data']['components'];
  $components = commerce_vat_components($components);
  if (empty($components)) {
    return 0;
  }
  // Get first element only. More than one VAT rate per item is not a common
  // case.
  $vat = reset($components);
  if (!isset($vat['price']['data']['vat_rate_info']['rate'])) {
    return 0;
  }
  // Return vat rate (in percents).
  return $vat['price']['data']['vat_rate_info']['rate'] * 100;
}

/**
 * Cancel inactive payment transactions.
 *
 * Cancel user transactions which might be considered as discarded.
 *
 * @param $order
 *   Order object.
 */
function _commerce_collector_cancel_deprecated_transactions($order) {
  // Find all invoices which can be activated.
  $query = db_select('commerce_payment_transaction', 't');
  $query->condition('t.order_id', $order->order_id);
  $query->condition('t.status', COMMERCE_PAYMENT_STATUS_PENDING);
  $query->condition(db_or()->isNull('t.remote_id')->condition('t.remote_id', ''));
  $query->fields('t');
  $result = $query->execute()->fetchAllKeyed();

  $transactions = entity_load('commerce_payment_transaction', array_keys($result));
  foreach ($transactions as $transaction) {
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    $transaction->message = t('Payment transaction has been discarded.');
    commerce_payment_transaction_save($transaction);
  }
}

/**
 * Error codes available for user.
 */
function _commerce_collector_fault_codes_messages($code = NULL) {
  $codes = array(
    'DENIED_TO_PURCHASE' => t('Unfortunately you were not approved for using invoice as your payment method. Please choose another payment method to complete your order.'),
    'CREDIT_CHECK_DENIED' => t('Unfortunately you were not approved for using invoice as your payment method. Please choose another payment method to complete your order.'),
    'RESERVATION_NOT_APPROVED' => t('Unfortunately you were not approved for using invoice as your payment method. Please choose another payment method to complete your order.'),
    'PURCHASE_AMOUNT_GREATER_THAN_MAX_CREDIT_AMOUNT' => t('The total value of your shopping cart is too large for the invoice payment method. Please remove some products from your cart or choose another payment method to complete your order.'),
    'INVALID_REGISTRATION_NUMBER' => t('The personal number you supplied is not correct. Please verify the number and try again.'),
    'AGREEMENT_RULES_VALIDATION_FAILED' => t('The personal information you supplied does not match with the information registered in the National Registry. Please verify your information and try again.'),
    'UNHANDLED_EXCEPTION' => t('Technical problem. Please try again later of choose another payment method for completing your order.'),
  );
  if (isset($codes[str_replace('s:', '', $code)])) {
    return $codes[str_replace('s:', '', $code)];
  }
  elseif (isset($code)) {
    return FALSE;
  }
  else {
    return $codes;
  }
}

/**
 * List of countries where Collector is available.
 */
function _commerce_collector_get_countries() {
  // Load locale.
  include_once DRUPAL_ROOT . '/includes/locale.inc';
  $available = array('NO', 'SE', 'DK', 'FI', 'DE', 'AT', 'NL');
  return array_intersect_key(country_get_list(), array_flip($available));
}

/**
 * Customer profile instances in commerce_order entities.
 *
 * @return array
 *   Return array of key value pairs
 */
function _commerce_collector_get_customer_profile_instances() {
  $instances = field_info_instances('commerce_order');
  $array = array(NULL => t('- Choose -'));

  foreach ($instances as $type => $instance) {
    foreach ($instance as $field_name => $field_instance) {
      $field = field_info_field($field_name);
      if ($field['type'] !== 'commerce_customer_profile_reference') {
        continue;
      }
      $array[$type][$field_name] = $field_instance['label'];
    }
  }

  return $array;
}

/**
 * List of available partial payment rates.
 */
function _commerce_collector_get_rates() {
  return array(
    'IF_3_001' => t('3 months, 0% interest rate, 95,- initial charge'),
    'IF_6_001' => t('6 months, 0% interest rate, 195,- initial charge'),
    'IF_12_001' => t('12 months, 0% interest rate, 295,- initial charge'),
    'AN_24_001' => t('24 months, 9,95% interest rate, 295,- initial charge'),
    'AN_36_001' => t('36 months, 14,95% interest rate, 395,- initial charge'),
  );
}

/**
 * Checks if an order contains a collector_fee line item.
 *
 * @param object $order
 *   The order object to check for a collector_fee line item.
 *
 * @return bool
 *   TRUE if the order contains a cot_fee line item.
 */
function _commerce_collector_line_item_exists($order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  foreach ($order_wrapper->commerce_line_items->getIterator() as $line_item_wrapper) {
    // If this line item is a cod_fee line item...
    if (isset($line_item_wrapper->type) && $line_item_wrapper->type->value() == 'collector_fee') {
      return TRUE;
    }
  }
  return FALSE;
}
